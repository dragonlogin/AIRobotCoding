#pragma once

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QVector>
#include <QString>

#include "core/DataModel.h"

/**
 * @brief ROS1 communication bridge layer
 *
 * Wraps all ROS1 API calls, runs in a dedicated thread,
 * and communicates with the main (UI) thread via Qt signals to avoid blocking.
 *
 * Subscribed topics:
 * - /joint_states (sensor_msgs/JointState)
 * - /wrench (geometry_msgs/WrenchStamped) - force sensor
 *
 * Published topics:
 * - /grinding_trajectory (trajectory_msgs/JointTrajectory)
 * - /grinding_command (std_msgs/String)
 *
 * Services:
 * - /emergency_stop (std_srvs/Trigger)
 */
class RosBridge : public QObject
{
    Q_OBJECT

public:
    explicit RosBridge(QObject* parent = nullptr);
    ~RosBridge();

    /// Connect to ROS Master
    bool connectToMaster(const QString& masterUri, const QString& nodeName = "airobot_grinding");

    /// Disconnect from ROS Master
    void disconnect();

    /// Returns true if currently connected
    bool isConnected() const { return m_connected; }

    /// Send a joint trajectory
    void sendTrajectory(const QVector<QVector<double>>& jointPositions,
                        const QVector<double>& timeFromStart);

    /// Send a single-point joint command
    void sendJointCommand(const double joints[6], double duration = 1.0);

    /// Send a Cartesian space motion command
    void sendCartesianCommand(double x, double y, double z,
                              double rx, double ry, double rz,
                              double duration = 1.0);

    /// Trigger emergency stop
    void emergencyStop();

    /// Send a custom command string
    void sendCommand(const QString& command);

    /// Get the current joint state
    RobotState currentState() const;

    /// Get the list of subscribed topics
    QVector<QPair<QString, QString>> subscribedTopics() const;

signals:
    /// Joint state updated
    void jointStateUpdated(const RobotState& state);

    /// Force sensor data updated
    void wrenchUpdated(double fx, double fy, double fz,
                       double tx, double ty, double tz);

    /// Connection state changed
    void connectionChanged(bool connected);

    /// Trajectory execution finished
    void trajectoryFinished(bool success);

    /// An error occurred
    void errorOccurred(const QString& error);

private:
    void rosSpinThread();

    bool m_connected = false;
    QThread* m_spinThread = nullptr;
    mutable QMutex m_mutex;
    RobotState m_currentState;

#ifdef HAS_ROS
    // ROS node handle, subscribers, and publishers declared here
    // ros::NodeHandle* m_nodeHandle = nullptr;
    // ros::Subscriber m_jointStateSub;
    // ros::Subscriber m_wrenchSub;
    // ros::Publisher m_trajectoryPub;
    // ros::Publisher m_commandPub;
    // ros::ServiceClient m_estopClient;
#endif
};
