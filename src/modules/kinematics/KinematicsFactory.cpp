#include "KinematicsFactory.h"
#include "KdlKinematics.h"
#include <QDebug>

std::unique_ptr<IKinematics> KinematicsFactory::create(const QString& robotType)
{
    // Future extension: switch to MoveItKinematics when HAS_ROS is defined
    // #ifdef HAS_ROS
    //     auto kin = std::make_unique<MoveItKinematics>();
    // #else
    auto kin = std::make_unique<KdlKinematics>();
    // #endif

    if (!kin->setRobotConfig(robotType)) {
        qWarning() << "KinematicsFactory: failed to create kinematics solver for robot" << robotType;
        return nullptr;
    }

    qDebug() << "KinematicsFactory: created" << kin->robotName()
             << "kinematics solver, backend:" << backendName();
    return kin;
}

QStringList KinematicsFactory::availableRobots()
{
    return {"ur5", "ur5e", "ur10"};
}

QString KinematicsFactory::backendName()
{
#ifdef HAS_KDL
    return "KDL (orocos)";
#elif defined(HAS_ROS)
    return "MoveIt!";
#else
    return "Built-in numerical IK (fallback mode)";
#endif
}
