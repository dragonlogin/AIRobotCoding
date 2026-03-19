#pragma once

#include <QVector>
#include "core/DataModel.h"

/**
 * @brief Path optimizer
 *
 * Post-processes the generated grinding path:
 * - Remove redundant points
 * - Feed rate planning (curvature-adaptive speed adjustment)
 * - Acceleration/deceleration transitions
 * - Adaptive pressure (adjust grinding pressure based on curvature)
 */
class PathOptimizer
{
public:
    /// Optimization parameters
    struct Parameters {
        double minPointDistance = 0.5;      // Minimum point spacing mm
        double maxPointDistance = 10.0;     // Maximum point spacing mm
        double maxFeedRate = 1000.0;       // Maximum feed rate mm/min
        double minFeedRate = 50.0;         // Minimum feed rate mm/min
        double acceleration = 500.0;       // Acceleration mm/s²
        double curvatureSpeedFactor = 0.5; // Curvature influence on speed
        double curvaturePressureFactor = 1.0; // Curvature influence on pressure
        double basePressure = 10.0;        // Base grinding pressure N
    };

    PathOptimizer() = default;

    void setParameters(const Parameters& params) { m_params = params; }

    /// Run all optimizations
    QVector<PathPoint> optimize(const QVector<PathPoint>& path);

    /// Remove redundant points (points too close together)
    QVector<PathPoint> removeRedundantPoints(const QVector<PathPoint>& path);

    /// Curvature-adaptive feed rate adjustment
    QVector<PathPoint> adaptiveFeedRate(const QVector<PathPoint>& path);

    /// Trapezoidal velocity profile planning
    QVector<PathPoint> trapezoidalVelocity(const QVector<PathPoint>& path);

    /// Curvature-adaptive pressure adjustment
    QVector<PathPoint> adaptivePressure(const QVector<PathPoint>& path);

private:
    /// Estimate local curvature at a path point
    double estimateCurvature(const QVector<PathPoint>& path, int index);

    Parameters m_params;
};
