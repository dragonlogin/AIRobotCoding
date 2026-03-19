#include "SurfaceAnalyzer.h"

#include <BRep_Tool.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepLProp_SLProps.hxx>
#include <Geom_Surface.hxx>
#include <TopAbs_Orientation.hxx>
#include <Precision.hxx>

void SurfaceAnalyzer::setFace(const TopoDS_Face& face)
{
    m_face = face;
}

void SurfaceAnalyzer::uvBounds(double& uMin, double& uMax, double& vMin, double& vMax) const
{
    BRepAdaptor_Surface adaptor(m_face);
    uMin = adaptor.FirstUParameter();
    uMax = adaptor.LastUParameter();
    vMin = adaptor.FirstVParameter();
    vMax = adaptor.LastVParameter();
}

SurfaceAnalyzer::SamplePoint SurfaceAnalyzer::evaluate(double u, double v) const
{
    SamplePoint pt;
    pt.u = u;
    pt.v = v;

    BRepAdaptor_Surface adaptor(m_face);
    BRepLProp_SLProps props(adaptor, u, v, 2, Precision::Confusion());

    // Position
    pt.position = adaptor.Value(u, v);

    // Normal
    if (props.IsNormalDefined()) {
        gp_Dir normal = props.Normal();
        if (m_face.Orientation() == TopAbs_REVERSED) {
            normal.Reverse();
        }
        pt.normal = gp_Vec(normal);
    }

    // Curvature
    if (props.IsCurvatureDefined()) {
        pt.minCurvature = props.MinCurvature();
        pt.maxCurvature = props.MaxCurvature();
    }

    return pt;
}

QVector<QVector<SurfaceAnalyzer::SamplePoint>> SurfaceAnalyzer::sampleGrid(
    int uCount, int vCount) const
{
    double uMin, uMax, vMin, vMax;
    uvBounds(uMin, uMax, vMin, vMax);

    QVector<QVector<SamplePoint>> grid(uCount);

    for (int i = 0; i < uCount; ++i) {
        grid[i].resize(vCount);
        double u = uMin + (uMax - uMin) * i / (uCount - 1);

        for (int j = 0; j < vCount; ++j) {
            double v = vMin + (vMax - vMin) * j / (vCount - 1);
            grid[i][j] = evaluate(u, v);
        }
    }

    return grid;
}

QVector<QVector<SurfaceAnalyzer::SamplePoint>> SurfaceAnalyzer::generateIsoLines(
    int lineCount, int pointsPerLine) const
{
    double uMin, uMax, vMin, vMax;
    uvBounds(uMin, uMax, vMin, vMax);

    QVector<QVector<SamplePoint>> isoLines(lineCount);

    for (int i = 0; i < lineCount; ++i) {
        isoLines[i].resize(pointsPerLine);

        // Iso-parameter line along the V direction (U fixed)
        double u = uMin + (uMax - uMin) * i / (lineCount - 1);

        for (int j = 0; j < pointsPerLine; ++j) {
            double v = vMin + (vMax - vMin) * j / (pointsPerLine - 1);
            isoLines[i][j] = evaluate(u, v);
        }
    }

    return isoLines;
}

gp_Vec SurfaceAnalyzer::toolNormal(double u, double v, double toolRadius) const
{
    SamplePoint pt = evaluate(u, v);

    // Base normal
    gp_Vec normal = pt.normal;
    if (normal.Magnitude() < Precision::Confusion()) {
        return gp_Vec(0, 0, 1);
    }
    normal.Normalize();

    // Curvature compensation: in concave regions the tool must account for the curvature radius.
    // If the curvature radius is smaller than the tool radius, the tool orientation must be adjusted.
    double maxAbsCurv = std::max(std::abs(pt.minCurvature), std::abs(pt.maxCurvature));
    if (maxAbsCurv > Precision::Confusion()) {
        double curvRadius = 1.0 / maxAbsCurv;
        if (curvRadius < toolRadius * 2.0) {
            // Concave region: apply additional offset along the normal direction.
            // A more sophisticated compensation algorithm is required in production.
        }
    }

    return normal;
}
