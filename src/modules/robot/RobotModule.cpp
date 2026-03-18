#include "RobotModule.h"
#include "core/EventBus.h"
#include "core/DataModel.h"

#include <QAction>

RobotModule::RobotModule(QObject* parent)
    : QObject(parent)
{
}

bool RobotModule::initialize()
{
    // 状态轮询定时器 (ROS 连接后启动)
    m_stateTimer = new QTimer(this);
    connect(m_stateTimer, &QTimer::timeout, this, [this]() {
        if (!m_rosConnected) return;

        // TODO: 通过 ROS 订阅获取实际关节状态
        // 此处为占位
        RobotState state;
        state.connected = true;
        state.statusText = "空闲";
        DataModel::instance()->updateRobotState(state);
    });

    // 监听连接请求
    connect(EventBus::instance(), &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            if (event == "robot.connect.request") {
                connectToRos(data.value("masterUri", "localhost:11311").toString());
            } else if (event == "robot.disconnect.request") {
                disconnectRos();
            } else if (event == "robot.estop") {
                emergencyStop();
            }
        });

    return true;
}

void RobotModule::shutdown()
{
    disconnectRos();
}

QList<QAction*> RobotModule::toolBarActions()
{
    QList<QAction*> actions;

    QAction* connectAction = new QAction("连接 ROS", this);
    connect(connectAction, &QAction::triggered, this, [this]() {
        connectToRos("localhost:11311");
    });
    actions.append(connectAction);

    QAction* eStopAction = new QAction("急停", this);
    eStopAction->setShortcut(QKeySequence("Escape"));
    connect(eStopAction, &QAction::triggered, this, &RobotModule::emergencyStop);
    actions.append(eStopAction);

    return actions;
}

QList<QAction*> RobotModule::menuActions()
{
    return {};
}

void RobotModule::connectToRos(const QString& masterUri)
{
    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("正在连接 ROS Master: %1").arg(masterUri)}
    });

#ifdef HAS_ROS
    // TODO: ros::init(), ros::NodeHandle
    // 订阅 /joint_states
    // 发布 /grinding_trajectory
#endif

    m_rosConnected = true;
    m_stateTimer->start(50);  // 20Hz 状态更新

    RobotState state;
    state.connected = true;
    state.statusText = "已连接";
    DataModel::instance()->updateRobotState(state);

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "ROS Master 连接成功"}
    });
}

void RobotModule::disconnectRos()
{
    m_stateTimer->stop();
    m_rosConnected = false;

    RobotState state;
    state.connected = false;
    state.statusText = "未连接";
    DataModel::instance()->updateRobotState(state);
}

void RobotModule::emergencyStop()
{
    EventBus::instance()->publish("log.message", {
        {"level", "WARN"},
        {"message", "紧急停止已触发！"}
    });

    // TODO: 发布急停指令到 ROS
}
