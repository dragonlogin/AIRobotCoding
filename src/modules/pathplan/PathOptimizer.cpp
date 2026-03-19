#include "PathOptimizer.h"

#include <QtMath>
#include <algorithm>

QVector<PathPoint> PathOptimizer::optimize(const QVector<PathPoint>& path)
{
    if (path.size() < 3) return path;

    QVector<PathPoint> result = path;
    result = removeRedundantPoints(result);
    result = adaptiveFeedRate(result);
    result = adaptivePressure(result);
    result = trapezoidalVelocity(result);

    return result;
}

QVector<PathPoint> PathOptimizer::removeRedundantPoints(
    const QVector<PathPoint>& path)
{
    if (path.size() < 2) return path;

    QVector<PathPoint> result;
    result.append(path.first());

    for (int i = 1; i < path.size() - 1; ++i) {
        QVector3D diff = path[i].position - result.last().position;
        double dist = static_cast<double>(diff.length());

        if (dist >= m_params.minPointDistance) {
            result.append(path[i]);
        }
    }

    // Always keep the last point
    result.append(path.last());
    return result;
}

QVector<PathPoint> PathOptimizer::adaptiveFeedRate(
    const QVector<PathPoint>& path)
{
    QVector<PathPoint> result = path;

    for (int i = 0; i < result.size(); ++i) {
        double curvature = estimateCurvature(result, i);

        // Higher curvature → lower speed
        // feedRate = maxFeed * (1 - factor * curvature / maxCurvature)
        double speedFactor = 1.0 - m_params.curvatureSpeedFactor *
                             std::min(curvature * 10.0, 1.0);
        speedFactor = qBound(0.1, speedFactor, 1.0);

        result[i].feedRate = m_params.minFeedRate +
            (m_params.maxFeedRate - m_params.minFeedRate) * speedFactor;
    }

    return result;
}

QVector<PathPoint> PathOptimizer::trapezoidalVelocity(
    const QVector<PathPoint>& path)
{
    if (path.size() < 3) return path;

    QVector<PathPoint> result = path;

    // Forward pass: acceleration constraint
    for (int i = 1; i < result.size(); ++i) {
        QVector3D diff = result[i].position - result[i - 1].position;
        double dist = static_cast<double>(diff.length());
        if (dist < 0.001) continue;

        // v² = v0² + 2*a*d
        double prevSpeed = result[i - 1].feedRate / 60.0;  // convert mm/min -> mm/s
        double maxSpeed = qSqrt(prevSpeed * prevSpeed +
                                2.0 * m_params.acceleration * dist);
        double maxFeedRate = maxSpeed * 60.0;  // convert back to mm/min

        result[i].feedRate = std::min(result[i].feedRate, maxFeedRate);
    }

    // Backward pass: deceleration constraint
    for (int i = result.size() - 2; i >= 0; --i) {
        QVector3D diff = result[i + 1].position - result[i].position;
        double dist = static_cast<double>(diff.length());
        if (dist < 0.001) continue;

        double nextSpeed = result[i + 1].feedRate / 60.0;
        double maxSpeed = qSqrt(nextSpeed * nextSpeed +
                                2.0 * m_params.acceleration * dist);
        double maxFeedRate = maxSpeed * 60.0;

        result[i].feedRate = std::min(result[i].feedRate, maxFeedRate);
    }

    // Reduce speed at start and end points
    result.first().feedRate = std::min(result.first().feedRate,
                                       m_params.maxFeedRate * 0.3);
    result.last().feedRate = std::min(result.last().feedRate,
                                      m_params.maxFeedRate * 0.3);

    return result;
}

QVector<PathPoint> PathOptimizer::adaptivePressure(
    const QVector<PathPoint>& path)
{
    QVector<PathPoint> result = path;

    for (int i = 0; i < result.size(); ++i) {
        double curvature = estimateCurvature(result, i);

        // Convex surface (positive curvature): increase pressure
        // Concave surface (negative curvature): decrease pressure
        // Flat surface (zero curvature): base pressure
        double pressureFactor = 1.0 + m_params.curvaturePressureFactor *
                                curvature * 5.0;
        pressureFactor = qBound(0.3, pressureFactor, 2.0);

        result[i].pressure = m_params.basePressure * pressureFactor;
    }

    return result;
}

double PathOptimizer::estimateCurvature(
    const QVector<PathPoint>& path, int index)
{
    if (index <= 0 || index >= path.size() - 1)
        return 0;

    // Estimate curvature from three points: k = 2 * |cross(p1-p0, p2-p0)| / (|p1-p0| * |p2-p0| * |p2-p1|)
    QVector3D p0 = path[index - 1].position;
    QVector3D p1 = path[index].position;
    QVector3D p2 = path[index + 1].position;

    QVector3D v1 = p1 - p0;
    QVector3D v2 = p2 - p0;

    QVector3D cross = QVector3D::crossProduct(v1, v2);
    double crossLen = static_cast<double>(cross.length());

    double a = static_cast<double>(v1.length());
    double b = static_cast<double>(v2.length());
    double c = static_cast<double>((p2 - p1).length());

    double denom = a * b * c;
    if (denom < 1e-10)
        return 0;

    return 2.0 * crossLen / denom;
}
