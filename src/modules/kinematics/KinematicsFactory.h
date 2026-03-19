#pragma once

#include "IKinematics.h"
#include <memory>
#include <QString>

/**
 * @brief Kinematics factory
 *
 * Automatically selects an implementation based on the build environment:
 *   HAS_KDL  -> KdlKinematics (KDL library)
 *   HAS_ROS  -> MoveItKinematics (future extension)
 *   otherwise -> KdlKinematics (fallback mode with built-in numerical IK)
 *
 * Usage:
 *   auto kin = KinematicsFactory::create("ur5");
 *   if (kin) { ... }
 */
class KinematicsFactory
{
public:
    static std::unique_ptr<IKinematics> create(const QString& robotType = "ur5");

    /// List robot models supported in the current environment
    static QStringList availableRobots();

    /// Name of the kinematics backend currently in use
    static QString backendName();
};
