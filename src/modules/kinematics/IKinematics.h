#pragma once

#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>
#include <array>

/// Joint-space waypoint
struct JointWaypoint
{
    std::array<double, 6> joints = {0, 0, 0, 0, 0, 0};
    double timeFromStart = 0.0;  // seconds
};

/// Cartesian-space pose (position + rotation matrix)
struct CartesianPose
{
    QVector3D position;
    QMatrix4x4 rotation;  // 3x3 rotation stored in the upper-left of a 4x4 matrix
};

/**
 * @brief Abstract kinematics interface
 *
 * macOS / no-ROS environment: implemented by KdlKinematics
 * Ubuntu / ROS environment: implemented by MoveItKinematics (future extension)
 *
 * Usage:
 *   auto kin = KinematicsFactory::create();
 *   kin->setRobotConfig("ur5");
 *   JointWaypoint wp = kin->inverseKinematics(pose, seedJoints);
 */
class IKinematics
{
public:
    virtual ~IKinematics() = default;

    /// Load robot configuration ("ur5", "ur10", "custom", etc.)
    virtual bool setRobotConfig(const QString& robotType) = 0;

    /// Forward kinematics: joint angles → end-effector pose
    virtual CartesianPose forwardKinematics(const std::array<double, 6>& joints) = 0;

    /// Inverse kinematics: end-effector pose → joint angles
    /// (seedJoints is the initial guess to improve convergence and solution continuity)
    virtual bool inverseKinematics(const CartesianPose& pose,
                                   const std::array<double, 6>& seedJoints,
                                   std::array<double, 6>& resultJoints) = 0;

    /// Batch IK: compute a joint trajectory for an entire path (ensuring solution continuity)
    virtual QVector<JointWaypoint> computeTrajectory(
        const QVector<CartesianPose>& poses,
        const std::array<double, 6>& startJoints,
        double feedRate) = 0;

    /// Check whether joint angles are within their limits
    virtual bool checkJointLimits(const std::array<double, 6>& joints) const = 0;

    /// Get the robot name
    virtual QString robotName() const = 0;
};
