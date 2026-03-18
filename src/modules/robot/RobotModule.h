#pragma once

#include "core/PluginInterface.h"
#include <QObject>
#include <QTimer>

/**
 * @brief 机器人模块 - ROS1 通信与机器人控制
 *
 * 职责：
 * - ROS Master 连接管理
 * - 关节状态订阅 (sensor_msgs/JointState)
 * - 运动指令发布 (trajectory_msgs)
 * - URDF 模型加载与可视化
 * - 急停功能
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

private:
    void connectToRos(const QString& masterUri);
    void disconnectRos();
    void emergencyStop();

    QTimer* m_stateTimer = nullptr;
    bool m_rosConnected = false;
};
