#pragma once

#include "core/PluginInterface.h"
#include <QObject>
#include <QTimer>

class RosBridge;

/**
 * @brief 机器人模块 - ROS1 通信与机器人控制
 *
 * 职责：
 * - ROS Master 连接管理（通过 RosBridge）
 * - 关节状态订阅 (sensor_msgs/JointState)
 * - 运动指令发布 (trajectory_msgs)
 * - URDF 模型加载与可视化
 * - 急停功能
 * - 模拟模式（无 ROS 环境时可用）
 */
class RobotModule : public QObject, public IModule
{
    Q_OBJECT
    Q_INTERFACES(IModule)

public:
    explicit RobotModule(QObject* parent = nullptr);

    QString moduleId() const override { return "robot"; }
    QString moduleName() const override { return "机器人控制"; }
    bool initialize() override;
    void shutdown() override;
    QList<QAction*> toolBarActions() override;
    QList<QAction*> menuActions() override;

    RosBridge* bridge() const { return m_bridge; }

private:
    void connectToRos(const QString& masterUri);
    void disconnectRos();
    void emergencyStop();
    void startSimulation();

    RosBridge* m_bridge = nullptr;
    QTimer* m_simTimer = nullptr;    // 模拟模式定时器
    double m_simTime = 0;
};
