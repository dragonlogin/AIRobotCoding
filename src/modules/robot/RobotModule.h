#pragma once

#include "core/PluginInterface.h"
#include <QObject>
#include <QTimer>

class RosBridge;

/**
 * @brief Robot module - ROS1 communication and robot control
 *
 * Responsibilities:
 * - ROS Master connection management (via RosBridge)
 * - Joint state subscription (sensor_msgs/JointState)
 * - Motion command publishing (trajectory_msgs)
 * - URDF model loading and visualization
 * - Emergency stop functionality
 * - Simulation mode (available when no ROS environment is present)
 */
class RobotModule : public QObject, public IModule
{
    Q_OBJECT
    Q_INTERFACES(IModule)
public:
    explicit RobotModule(QObject* parent = nullptr);
    QString moduleId() const override { return QString("robot"); }
    QString moduleName() const override { return QStringLiteral("Robot Control"); }
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
    QTimer* m_simTimer = nullptr;    // Simulation mode timer
    double m_simTime = 0;
};
