#pragma once

#include "IKinematics.h"
#include <memory>
#include <QString>

/**
 * @brief 运动学工厂
 *
 * 根据编译环境自动选择实现：
 *   HAS_KDL  → KdlKinematics（KDL 库）
 *   HAS_ROS  → MoveItKinematics（未来扩展）
 *   否则     → KdlKinematics（降级模式，内置数值 IK）
 *
 * 用法：
 *   auto kin = KinematicsFactory::create("ur5");
 *   if (kin) { ... }
 */
class KinematicsFactory
{
public:
    static std::unique_ptr<IKinematics> create(const QString& robotType = "ur5");

    /// 列出当前环境支持的机器人型号
    static QStringList availableRobots();

    /// 当前使用的运动学后端名称
    static QString backendName();
};
