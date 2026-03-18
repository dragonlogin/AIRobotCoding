#pragma once

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QVector>
#include <QString>

#include "core/DataModel.h"

/**
 * @brief ROS1 通信桥接层
 *
 * 封装所有 ROS1 API 调用，运行在独立线程中，
 * 通过 Qt 信号与主线程（UI）通信，避免阻塞。
 *
 * 订阅话题：
 * - /joint_states (sensor_msgs/JointState)
 * - /wrench (geometry_msgs/WrenchStamped) - 力传感器
 *
 * 发布话题：
 * - /grinding_trajectory (trajectory_msgs/JointTrajectory)
 * - /grinding_command (std_msgs/String)
 *
 * 服务：
 * - /emergency_stop (std_srvs/Trigger)
 */
class RosBridge : public QObject
{
    Q_OBJECT

public:
    explicit RosBridge(QObject* parent = nullptr);
    ~RosBridge();

    /// 连接到 ROS Master
    bool connectToMaster(const QString& masterUri, const QString& nodeName = "airobot_grinding");

    /// 断开连接
    void disconnect();

    /// 是否已连接
    bool isConnected() const { return m_connected; }

    /// 发送关节轨迹
    void sendTrajectory(const QVector<QVector<double>>& jointPositions,
                        const QVector<double>& timeFromStart);

    /// 发送单点运动指令
    void sendJointCommand(const double joints[6], double duration = 1.0);

    /// 发送笛卡尔空间运动指令
    void sendCartesianCommand(double x, double y, double z,
                              double rx, double ry, double rz,
                              double duration = 1.0);

    /// 急停
    void emergencyStop();

    /// 发送自定义指令
    void sendCommand(const QString& command);

    /// 获取当前关节状态
    RobotState currentState() const;

    /// 获取已订阅的话题列表
    QVector<QPair<QString, QString>> subscribedTopics() const;

signals:
    /// 关节状态更新
    void jointStateUpdated(const RobotState& state);

    /// 力传感器数据更新
    void wrenchUpdated(double fx, double fy, double fz,
                       double tx, double ty, double tz);

    /// 连接状态变化
    void connectionChanged(bool connected);

    /// 轨迹执行完成
    void trajectoryFinished(bool success);

    /// 错误发生
    void errorOccurred(const QString& error);

private:
    void rosSpinThread();

    bool m_connected = false;
    QThread* m_spinThread = nullptr;
    mutable QMutex m_mutex;
    RobotState m_currentState;

#ifdef HAS_ROS
    // ROS 句柄和订阅者/发布者将在此声明
    // ros::NodeHandle* m_nodeHandle = nullptr;
    // ros::Subscriber m_jointStateSub;
    // ros::Subscriber m_wrenchSub;
    // ros::Publisher m_trajectoryPub;
    // ros::Publisher m_commandPub;
    // ros::ServiceClient m_estopClient;
#endif
};
