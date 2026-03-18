#include "KinematicsFactory.h"
#include "KdlKinematics.h"
#include <QDebug>

std::unique_ptr<IKinematics> KinematicsFactory::create(const QString& robotType)
{
    // 未来扩展：HAS_ROS 时可切换到 MoveItKinematics
    // #ifdef HAS_ROS
    //     auto kin = std::make_unique<MoveItKinematics>();
    // #else
    auto kin = std::make_unique<KdlKinematics>();
    // #endif

    if (!kin->setRobotConfig(robotType)) {
        qWarning() << "KinematicsFactory: 无法创建机器人" << robotType << "的运动学解算器";
        return nullptr;
    }

    qDebug() << "KinematicsFactory: 已创建" << kin->robotName()
             << "运动学解算器，后端:" << backendName();
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
    return "内置数值 IK（降级模式）";
#endif
}
