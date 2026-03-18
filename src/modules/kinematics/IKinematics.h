#pragma once

#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>
#include <array>

/// 关节空间路径点
struct JointWaypoint
{
    std::array<double, 6> joints = {0, 0, 0, 0, 0, 0};
    double timeFromStart = 0.0;  // 秒
};

/// 笛卡尔空间位姿（位置 + 旋转矩阵）
struct CartesianPose
{
    QVector3D position;
    QMatrix4x4 rotation;  // 3x3 旋转，存在 4x4 矩阵左上角
};

/**
 * @brief 运动学抽象接口
 *
 * macOS/无ROS 环境：KdlKinematics 实现
 * Ubuntu/ROS 环境：MoveItKinematics 实现（未来扩展）
 *
 * 用法：
 *   auto kin = KinematicsFactory::create();
 *   kin->setRobotConfig("ur5");
 *   JointWaypoint wp = kin->inverseKinematics(pose, seedJoints);
 */
class IKinematics
{
public:
    virtual ~IKinematics() = default;

    /// 加载机器人配置（"ur5", "ur10", "custom" 等）
    virtual bool setRobotConfig(const QString& robotType) = 0;

    /// 正运动学：关节角 → 末端位姿
    virtual CartesianPose forwardKinematics(const std::array<double, 6>& joints) = 0;

    /// 逆运动学：末端位姿 → 关节角（seedJoints 作为初始猜测，提高收敛速度和解的连续性）
    virtual bool inverseKinematics(const CartesianPose& pose,
                                   const std::array<double, 6>& seedJoints,
                                   std::array<double, 6>& resultJoints) = 0;

    /// 批量逆解：为整条路径生成关节轨迹（保证解的连续性）
    virtual QVector<JointWaypoint> computeTrajectory(
        const QVector<CartesianPose>& poses,
        const std::array<double, 6>& startJoints,
        double feedRate) = 0;

    /// 检查关节角是否在限位内
    virtual bool checkJointLimits(const std::array<double, 6>& joints) const = 0;

    /// 获取机器人名称
    virtual QString robotName() const = 0;
};
