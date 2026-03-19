#include "KdlKinematics.h"
#include <QtMath>
#include <QDebug>
#include <cmath>

// ============================================================================
// Utility: build grinding pose from position + surface normal
// ============================================================================
CartesianPose KdlKinematics::poseFromNormal(const QVector3D& position,
                                             const QVector3D& normal,
                                             const QVector3D& pathDirection)
{
    CartesianPose pose;
    pose.position = position;

    // Tool Z-axis: along the surface normal (tool perpendicular to grinding surface)
    QVector3D z = -normal.normalized();  // Tool points toward the surface

    // Tool X-axis: projection of the path direction onto the surface tangent plane
    QVector3D x = pathDirection - QVector3D::dotProduct(pathDirection, z) * z;
    if (x.length() < 1e-6f) {
        // If path direction is parallel to the normal, pick an arbitrary perpendicular direction
        x = QVector3D(1, 0, 0);
        if (std::abs(QVector3D::dotProduct(x, z)) > 0.9f)
            x = QVector3D(0, 1, 0);
        x = x - QVector3D::dotProduct(x, z) * z;
    }
    x.normalize();

    // Tool Y-axis: right-hand coordinate system
    QVector3D y = QVector3D::crossProduct(z, x).normalized();

    // Store in rotation matrix (column vectors are the x, y, z axes)
    pose.rotation.setToIdentity();
    pose.rotation(0, 0) = x.x(); pose.rotation(0, 1) = y.x(); pose.rotation(0, 2) = z.x();
    pose.rotation(1, 0) = x.y(); pose.rotation(1, 1) = y.y(); pose.rotation(1, 2) = z.y();
    pose.rotation(2, 0) = x.z(); pose.rotation(2, 1) = y.z(); pose.rotation(2, 2) = z.z();

    return pose;
}

// ============================================================================
// setRobotConfig
// ============================================================================
bool KdlKinematics::setRobotConfig(const QString& robotType)
{
    if (!RobotConfigLibrary::get(robotType, m_config)) {
        qWarning() << "KdlKinematics: unsupported robot model:" << robotType;
        return false;
    }

#ifdef HAS_KDL
    buildKdlChain();
#endif

    m_initialized = true;
    qDebug() << "KdlKinematics: robot configuration loaded:" << m_config.name;
    return true;
}

// ============================================================================
// Forward kinematics
// ============================================================================
CartesianPose KdlKinematics::forwardKinematics(const std::array<double, 6>& joints)
{
    CartesianPose result;

#ifdef HAS_KDL
    if (!m_fkSolver) return result;

    KDL::JntArray q = toKdlJoints(joints);
    KDL::Frame frame;
    if (m_fkSolver->JntToCart(q, frame) >= 0) {
        result = fromKdlFrame(frame);
    }
#else
    // Without KDL: manually compute DH forward kinematics
    QMatrix4x4 T;
    T.setToIdentity();
    for (int i = 0; i < 6; ++i) {
        T = T * dhTransform(m_config.a[i],
                            m_config.d[i],
                            m_config.alpha[i],
                            joints[i] + m_config.thetaOffset[i]);
    }
    result.position = QVector3D(T(0,3), T(1,3), T(2,3));
    result.rotation = T;
#endif

    return result;
}

// ============================================================================
// Inverse kinematics
// ============================================================================
bool KdlKinematics::inverseKinematics(const CartesianPose& pose,
                                       const std::array<double, 6>& seedJoints,
                                       std::array<double, 6>& resultJoints)
{
#ifdef HAS_KDL
    if (!m_ikSolver) return false;

    KDL::JntArray q_init = toKdlJoints(seedJoints);
    KDL::JntArray q_out(6);
    KDL::Frame target = toKdlFrame(pose);

    int ret = m_ikSolver->CartToJnt(q_init, target, q_out);
    if (ret < 0) {
        return false;
    }

    for (int i = 0; i < 6; ++i)
        resultJoints[i] = q_out(i);

    return checkJointLimits(resultJoints);
#else
    // Without KDL: numerical iterative IK (Jacobian pseudo-inverse)
    // For development/debugging only; limited accuracy
    std::array<double, 6> q = seedJoints;
    const int maxIter = 200;
    const double eps = 0.01;  // 1 mm position accuracy

    for (int iter = 0; iter < maxIter; ++iter) {
        CartesianPose current = forwardKinematics(q);
        QVector3D posErr = pose.position - current.position;

        if (posErr.length() < eps) {
            resultJoints = q;
            return checkJointLimits(resultJoints);
        }

        // Numerical Jacobian (position component only; orientation not handled here)
        const double dq = 1e-5;
        for (int j = 0; j < 6; ++j) {
            std::array<double, 6> qp = q;
            qp[j] += dq;
            CartesianPose cp = forwardKinematics(qp);
            QVector3D col = (cp.position - current.position) / dq;
            // Simplified: correct position only
            q[j] += 0.1 * QVector3D::dotProduct(col, posErr) / (col.lengthSquared() + 1e-6);
        }
    }

    resultJoints = q;
    qWarning() << "KdlKinematics: IK iteration did not converge (no-KDL fallback mode)";
    return false;
#endif
}

// ============================================================================
// Batch trajectory computation
// ============================================================================
QVector<JointWaypoint> KdlKinematics::computeTrajectory(
    const QVector<CartesianPose>& poses,
    const std::array<double, 6>& startJoints,
    double feedRate)
{
    QVector<JointWaypoint> trajectory;
    if (poses.isEmpty()) return trajectory;

    std::array<double, 6> prevJoints = startJoints;
    double t = 0.0;

    // Starting waypoint
    JointWaypoint wp0;
    wp0.joints = startJoints;
    wp0.timeFromStart = 0.0;
    trajectory.append(wp0);

    int failCount = 0;

    for (int i = 0; i < poses.size(); ++i) {
        std::array<double, 6> q;
        bool ok = inverseKinematics(poses[i], prevJoints, q);

        if (!ok) {
            ++failCount;
            // On IK failure, reuse the previous solution to avoid trajectory discontinuity
            q = prevJoints;
        }

        // Estimate time from distance between points
        CartesianPose prevPose = forwardKinematics(prevJoints);
        double dist = (poses[i].position - prevPose.position).length();
        double dt = (feedRate > 0) ? (dist / feedRate * 60.0) : 0.1;
        t += std::max(dt, 0.01);

        JointWaypoint wp;
        wp.joints = q;
        wp.timeFromStart = t;
        trajectory.append(wp);

        prevJoints = q;
    }

    if (failCount > 0) {
        qWarning() << "KdlKinematics: IK solve failed for" << failCount << "points in trajectory";
    }

    return trajectory;
}

// ============================================================================
// Joint limit check
// ============================================================================
bool KdlKinematics::checkJointLimits(const std::array<double, 6>& joints) const
{
    for (int i = 0; i < 6; ++i) {
        if (joints[i] < m_config.jointMin[i] || joints[i] > m_config.jointMax[i])
            return false;
    }
    return true;
}

// ============================================================================
// KDL-specific implementation
// ============================================================================
#ifdef HAS_KDL

void KdlKinematics::buildKdlChain()
{
    m_chain = KDL::Chain();

    for (int i = 0; i < 6; ++i) {
        double a     = m_config.a[i] * 1e-3;  // mm -> m
        double d     = m_config.d[i] * 1e-3;
        double alpha = m_config.alpha[i];
        double offset = m_config.thetaOffset[i];

        m_chain.addSegment(KDL::Segment(
            KDL::Joint(KDL::Joint::RotZ),
            KDL::Frame::DH(a, alpha, d, offset)));
    }

    m_fkSolver  = std::make_unique<KDL::ChainFkSolverPos_recursive>(m_chain);
    m_ikVelSolver = std::make_unique<KDL::ChainIkSolverVel_pinv>(m_chain);
    m_ikSolver  = std::make_unique<KDL::ChainIkSolverPos_LMA>(m_chain);
}

KDL::Frame KdlKinematics::toKdlFrame(const CartesianPose& pose) const
{
    // Position mm -> m
    KDL::Vector pos(pose.position.x() * 1e-3,
                    pose.position.y() * 1e-3,
                    pose.position.z() * 1e-3);

    // Rotation matrix
    KDL::Rotation rot(
        pose.rotation(0,0), pose.rotation(0,1), pose.rotation(0,2),
        pose.rotation(1,0), pose.rotation(1,1), pose.rotation(1,2),
        pose.rotation(2,0), pose.rotation(2,1), pose.rotation(2,2));

    return KDL::Frame(rot, pos);
}

CartesianPose KdlKinematics::fromKdlFrame(const KDL::Frame& frame) const
{
    CartesianPose pose;
    // Position m -> mm
    pose.position = QVector3D(
        frame.p.x() * 1e3,
        frame.p.y() * 1e3,
        frame.p.z() * 1e3);

    // Rotation matrix
    pose.rotation.setToIdentity();
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            pose.rotation(r, c) = frame.M(r, c);

    return pose;
}

KDL::JntArray KdlKinematics::toKdlJoints(const std::array<double, 6>& joints) const
{
    KDL::JntArray q(6);
    for (int i = 0; i < 6; ++i)
        q(i) = joints[i];
    return q;
}

#else  // DH matrix fallback when KDL is not available

QMatrix4x4 KdlKinematics::dhTransform(double a, double d, double alpha, double theta) const
{
    float ca = std::cos(alpha), sa = std::sin(alpha);
    float ct = std::cos(theta), st = std::sin(theta);
    float af = a * 1e-3f;  // mm -> m (for simulation display; mm may also be kept as-is)
    float df = d * 1e-3f;

    QMatrix4x4 T(
         ct,   -st,    0,   af,
         st*ca, ct*ca, -sa, -sa*df,
         st*sa, ct*sa,  ca,  ca*df,
         0,     0,      0,   1);
    return T;
}

#endif
