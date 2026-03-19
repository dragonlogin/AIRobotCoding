#include "RosBridge.h"
#include "core/EventBus.h"

#include <QDebug>

#ifdef HAS_ROS
#include <ros/ros.h>
#include <sensor_msgs/JointState.h>
#include <geometry_msgs/WrenchStamped.h>
#include <trajectory_msgs/JointTrajectory.h>
#include <std_msgs/String.h>
#include <std_srvs/Trigger.h>
#endif

RosBridge::RosBridge(QObject* parent)
    : QObject(parent)
{
}

RosBridge::~RosBridge()
{
    disconnect();
}

bool RosBridge::connectToMaster(const QString& masterUri, const QString& nodeName)
{
#ifdef HAS_ROS
    // Set ROS Master URI
    std::string uri = masterUri.toStdString();
    setenv("ROS_MASTER_URI", (std::string("http://") + uri).c_str(), 1);

    // Initialize ROS node
    int argc = 0;
    char** argv = nullptr;
    ros::init(argc, argv, nodeName.toStdString(), ros::init_options::NoSigintHandler);

    if (!ros::master::check()) {
        emit errorOccurred(QString("Failed to connect to ROS Master: %1").arg(masterUri));
        return false;
    }

    ros::NodeHandle* nh = new ros::NodeHandle();

    // --- Subscribe to joint states ---
    ros::Subscriber jointSub = nh->subscribe<sensor_msgs::JointState>(
        "/joint_states", 10,
        [this](const sensor_msgs::JointState::ConstPtr& msg) {
            QMutexLocker locker(&m_mutex);

            RobotState state;
            state.connected = true;
            state.statusText = "Running";

            // Update joint values
            for (size_t i = 0; i < std::min(msg->position.size(), size_t(6)); ++i) {
                state.joints[i] = msg->position[i] * 180.0 / M_PI;  // rad -> deg
            }

            m_currentState = state;

            // Emit signal to main thread
            QMetaObject::invokeMethod(this, [this, state]() {
                emit jointStateUpdated(state);
            }, Qt::QueuedConnection);
        });

    // --- Subscribe to force sensor ---
    ros::Subscriber wrenchSub = nh->subscribe<geometry_msgs::WrenchStamped>(
        "/wrench", 10,
        [this](const geometry_msgs::WrenchStamped::ConstPtr& msg) {
            QMetaObject::invokeMethod(this, [this,
                fx = msg->wrench.force.x,
                fy = msg->wrench.force.y,
                fz = msg->wrench.force.z,
                tx = msg->wrench.torque.x,
                ty = msg->wrench.torque.y,
                tz = msg->wrench.torque.z]() {
                emit wrenchUpdated(fx, fy, fz, tx, ty, tz);
            }, Qt::QueuedConnection);
        });

    // --- Publishers ---
    // m_trajectoryPub = nh->advertise<trajectory_msgs::JointTrajectory>(
    //     "/grinding_trajectory", 1);
    // m_commandPub = nh->advertise<std_msgs::String>("/grinding_command", 1);
    // m_estopClient = nh->serviceClient<std_srvs::Trigger>("/emergency_stop");

    // --- Start ROS spin thread ---
    m_spinThread = new QThread(this);
    QObject* worker = new QObject();
    worker->moveToThread(m_spinThread);
    connect(m_spinThread, &QThread::started, [nh]() {
        ros::Rate rate(50);  // 50Hz
        while (ros::ok()) {
            ros::spinOnce();
            rate.sleep();
        }
        delete nh;
    });
    connect(m_spinThread, &QThread::finished, worker, &QObject::deleteLater);
    m_spinThread->start();

    m_connected = true;
    emit connectionChanged(true);

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("ROS Master connected: %1").arg(masterUri)}
    });

    return true;

#else
    // No ROS environment — fall back to simulation mode
    Q_UNUSED(nodeName)

    EventBus::instance()->publish("log.message", {
        {"level", "WARN"},
        {"message", QString("ROS not enabled, entering simulation mode (Master: %1)").arg(masterUri)}
    });

    m_connected = true;

    // Simulated joint state
    m_currentState.connected = true;
    m_currentState.statusText = QStringLiteral("Simulation Mode");

    emit connectionChanged(true);
    emit jointStateUpdated(m_currentState);

    return true;
#endif
}

void RosBridge::disconnect()
{
#ifdef HAS_ROS
    if (m_spinThread && m_spinThread->isRunning()) {
        ros::shutdown();
        m_spinThread->quit();
        m_spinThread->wait(3000);
    }
#endif

    m_connected = false;
    m_currentState = RobotState();
    emit connectionChanged(false);
}

void RosBridge::sendTrajectory(
    const QVector<QVector<double>>& jointPositions,
    const QVector<double>& timeFromStart)
{
#ifdef HAS_ROS
    if (!m_connected) return;

    trajectory_msgs::JointTrajectory traj;
    traj.joint_names = {"joint_1", "joint_2", "joint_3",
                        "joint_4", "joint_5", "joint_6"};

    for (int i = 0; i < jointPositions.size(); ++i) {
        trajectory_msgs::JointTrajectoryPoint point;
        for (int j = 0; j < std::min(jointPositions[i].size(), 6); ++j) {
            point.positions.push_back(
                jointPositions[i][j] * M_PI / 180.0);  // deg -> rad
        }
        point.time_from_start = ros::Duration(timeFromStart[i]);
        traj.points.push_back(point);
    }

    // m_trajectoryPub.publish(traj);

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("Trajectory sent: %1 waypoints").arg(jointPositions.size())}
    });
#else
    Q_UNUSED(jointPositions)
    Q_UNUSED(timeFromStart)
    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "[Simulation] Trajectory sent"}
    });
#endif
}

void RosBridge::sendJointCommand(const double joints[6], double duration)
{
    QVector<QVector<double>> positions(1);
    positions[0].resize(6);
    for (int i = 0; i < 6; ++i) {
        positions[0][i] = joints[i];
    }
    QVector<double> times = {duration};
    sendTrajectory(positions, times);
}

void RosBridge::sendCartesianCommand(
    double x, double y, double z,
    double rx, double ry, double rz,
    double duration)
{
    Q_UNUSED(x) Q_UNUSED(y) Q_UNUSED(z)
    Q_UNUSED(rx) Q_UNUSED(ry) Q_UNUSED(rz)
    Q_UNUSED(duration)

    // TODO: Convert to joint-space command via inverse kinematics,
    // or use MoveIt's Cartesian path planning service.

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("[Cartesian] Target: (%1, %2, %3) Orientation: (%4, %5, %6)")
            .arg(x, 0, 'f', 2).arg(y, 0, 'f', 2).arg(z, 0, 'f', 2)
            .arg(rx, 0, 'f', 2).arg(ry, 0, 'f', 2).arg(rz, 0, 'f', 2)}
    });
}

void RosBridge::emergencyStop()
{
#ifdef HAS_ROS
    if (m_connected) {
        std_srvs::Trigger srv;
        // m_estopClient.call(srv);

        // Also publish a stop command
        std_msgs::String msg;
        msg.data = "EMERGENCY_STOP";
        // m_commandPub.publish(msg);
    }
#endif

    EventBus::instance()->publish("log.message", {
        {"level", "WARN"},
        {"message", "Emergency stop command sent!"}
    });

    QMutexLocker locker(&m_mutex);
    m_currentState.moving = false;
    m_currentState.statusText = "Emergency Stop";
}

void RosBridge::sendCommand(const QString& command)
{
#ifdef HAS_ROS
    if (!m_connected) return;

    std_msgs::String msg;
    msg.data = command.toStdString();
    // m_commandPub.publish(msg);
#else
    Q_UNUSED(command)
#endif

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("Command sent: %1").arg(command)}
    });
}

RobotState RosBridge::currentState() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentState;
}

QVector<QPair<QString, QString>> RosBridge::subscribedTopics() const
{
    return {
        {"/joint_states", "sensor_msgs/JointState"},
        {"/wrench", "geometry_msgs/WrenchStamped"},
        {"/grinding_trajectory", "trajectory_msgs/JointTrajectory"},
        {"/grinding_command", "std_msgs/String"}
    };
}
