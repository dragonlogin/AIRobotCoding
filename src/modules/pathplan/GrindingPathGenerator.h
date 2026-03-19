#pragma once

#include <QVector>
#include <QVector3D>
#include "core/DataModel.h"

#include <TopoDS_Face.hxx>

/**
 * @brief Grinding path generator
 *
 * Generates a sequence of path points from surface geometry and grinding parameters.
 *
 * Supported path strategies:
 * - Raster: equidistant row scanning along the U direction
 * - Spiral: outward spiral from the centre
 * - Contour: scanning along iso-parameter contour lines
 * - Zigzag: raster scanning with alternating row directions
 */
class GrindingPathGenerator
{
public:
    /// Path generation strategy
    enum Strategy {
        Raster,     // Raster scan
        Zigzag,     // Zigzag scan
        Spiral,     // Spiral
        Contour     // Contour lines
    };

    /// Path generation parameters
    struct Parameters {
        Strategy strategy = Zigzag;
        double stepOver = 2.0;       // Step-over distance mm
        double feedRate = 500.0;     // Feed rate mm/min
        double pressure = 10.0;      // Grinding pressure N
        double toolRadius = 25.0;    // Tool radius mm
        double safeHeight = 20.0;    // Safe height mm (tool lift distance)
        double approachAngle = 0.0;  // Approach angle deg
        int pointDensity = 100;      // Sample points per line
        bool smoothPath = true;      // Enable path smoothing
    };

    GrindingPathGenerator() = default;

    /// Set the target face
    void setFace(const TopoDS_Face& face);

    /// Set parameters
    void setParameters(const Parameters& params) { m_params = params; }

    /// Generate the path
    QVector<PathPoint> generate();

    /// Get generation statistics
    struct Statistics {
        int totalPoints = 0;
        double totalLength = 0;     // mm
        double estimatedTime = 0;   // seconds
        double coverageArea = 0;    // mm²
    };
    Statistics statistics() const { return m_stats; }

private:
    QVector<PathPoint> generateRaster();
    QVector<PathPoint> generateZigzag();
    QVector<PathPoint> generateSpiral();
    QVector<PathPoint> generateContour();

    /// Path smoothing (B-spline interpolation)
    QVector<PathPoint> smoothPath(const QVector<PathPoint>& path);

    /// Add approach and retract moves
    QVector<PathPoint> addApproachRetract(const QVector<PathPoint>& path);

    /// Compute the distance between two path points
    static double distance(const PathPoint& a, const PathPoint& b);

    TopoDS_Face m_face;
    Parameters m_params;
    Statistics m_stats;
};
