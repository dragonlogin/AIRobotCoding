#pragma once

#include <QVector>
#include <QVector3D>
#include <TopoDS_Face.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

/**
 * @brief Surface analyzer - provides grinding-related surface geometry analysis
 *
 * Responsibilities:
 * - UV parameter domain sampling
 * - Normal vector field computation
 * - Curvature field computation
 * - Surface discretization (provides data for path planning)
 */
class SurfaceAnalyzer
{
public:
    struct SamplePoint {
        gp_Pnt position;
        gp_Vec normal;
        double minCurvature = 0;
        double maxCurvature = 0;
        double u = 0;
        double v = 0;
    };

    SurfaceAnalyzer() = default;

    /// Set the face to analyze
    void setFace(const TopoDS_Face& face);

    /// Get the UV parameter domain bounds
    void uvBounds(double& uMin, double& uMax, double& vMin, double& vMax) const;

    /// Evaluate the surface at the given UV coordinates
    SamplePoint evaluate(double u, double v) const;

    /// Uniformly sample the surface and return a grid of sample points
    QVector<QVector<SamplePoint>> sampleGrid(int uCount, int vCount) const;

    /// Generate iso-parameter lines along the U direction (for grinding row paths)
    QVector<QVector<SamplePoint>> generateIsoLines(
        int lineCount, int pointsPerLine) const;

    /// Compute the tool contact normal at the given point (accounting for tool radius compensation)
    gp_Vec toolNormal(double u, double v, double toolRadius) const;

private:
    TopoDS_Face m_face;
};
