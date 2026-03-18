#pragma once

#include "IKinematics.h"
#include "RobotConfig.h"

#ifdef HAS_KDL
#include <kdl/chain.hpp>
#include <kdl/chainfksolverpos_recursive.hpp>
#include <kdl/chainiksolvervel_pinv.hpp>
#include <kdl/chainiksolverpos_nr.hpp>
#include <kdl/chainiksolverpos_lma.hpp>
#include <kdl/jntarray.hpp>
#include <kdl/frames.hpp>
#include <memory>
#endif

/**
 * @brief 基于 KDL 的运动学实现
 *
 * 无 KDL 时自动降级为简化数值 IK（仅供开发调试）
 *
 * 编译：
 *   有 KDL（推荐）：brew install orocos-kdl
 *   无 KDL：降级为内置简化解算器
 */
class KdlKinematics : public IKinematics
{
public:
    KdlKinematics() = default;
    ~KdlKinematics() override = default;

    bool setRobotConfig(const QString& robotType) override;

    CartesianPose forwardKinematics(const std::array<double, 6>& joints) override;

    bool inverseKinematics(const CartesianPose& pose,
                           const std::array<double, 6>& seedJoints,
                           std::array<double, 6>& resultJoints) override;

    QVector<JointWaypoint> computeTrajectory(
        const QVector<CartesianPose>& poses,
        const std::array<double, 6>& startJoints,
        double feedRate) override;

    bool checkJointLimits(const std::array<double, 6>& joints) const override;

    QString robotName() const override { return m_config.name; }

public:
    /// 从位置 + 法向 构建打磨末端位姿（工具 Z 轴垂直曲面）
    static CartesianPose poseFromNormal(const QVector3D& position,
                                        const QVector3D& normal,
                                        const QVector3D& pathDirection);

private:

#ifdef HAS_KDL
    void buildKdlChain();
    KDL::Frame toKdlFrame(const CartesianPose& pose) const;
    CartesianPose fromKdlFrame(const KDL::Frame& frame) const;
    KDL::JntArray toKdlJoints(const std::array<double, 6>& joints) const;

    KDL::Chain m_chain;
    std::unique_ptr<KDL::ChainFkSolverPos_recursive> m_fkSolver;
    std::unique_ptr<KDL::ChainIkSolverVel_pinv>      m_ikVelSolver;
    std::unique_ptr<KDL::ChainIkSolverPos_LMA>        m_ikSolver;
#else
    /// 无 KDL 时：用 DH 矩阵手动计算正运动学
    QMatrix4x4 dhTransform(double a, double d, double alpha, double theta) const;
#endif

    RobotDHConfig m_config;
    bool m_initialized = false;

    friend class KinematicsFactory;
};
