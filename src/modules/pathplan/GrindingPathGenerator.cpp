#include "GrindingPathGenerator.h"

#include <BRepAdaptor_Surface.hxx>
#include <BRepLProp_SLProps.hxx>
#include <Precision.hxx>

#include <QtMath>

void GrindingPathGenerator::setFace(const TopoDS_Face& face)
{
    m_face = face;
}

QVector<PathPoint> GrindingPathGenerator::generate()
{
    if (m_face.IsNull())
        return {};

    QVector<PathPoint> path;

    switch (m_params.strategy) {
    case Raster:
        path = generateRaster();
        break;
    case Zigzag:
        path = generateZigzag();
        break;
    case Spiral:
        path = generateSpiral();
        break;
    case Contour:
        path = generateContour();
        break;
    }

    // 平滑处理
    if (m_params.smoothPath && path.size() > 3) {
        path = smoothPath(path);
    }

    // 添加进退刀
    path = addApproachRetract(path);

    // 统计
    m_stats.totalPoints = path.size();
    m_stats.totalLength = 0;
    for (int i = 1; i < path.size(); ++i) {
        m_stats.totalLength += distance(path[i - 1], path[i]);
    }
    m_stats.estimatedTime = (m_stats.totalLength / m_params.feedRate) * 60.0;

    return path;
}

// ============================================================================
// 栅格扫描路径
// ============================================================================
QVector<PathPoint> GrindingPathGenerator::generateRaster()
{
    QVector<PathPoint> path;

    BRepAdaptor_Surface adaptor(m_face);
    double uMin = adaptor.FirstUParameter();
    double uMax = adaptor.LastUParameter();
    double vMin = adaptor.FirstVParameter();
    double vMax = adaptor.LastVParameter();

    // 根据行距计算行数
    // 先估算 U 方向的物理长度
    gp_Pnt p1 = adaptor.Value(uMin, (vMin + vMax) / 2.0);
    gp_Pnt p2 = adaptor.Value(uMax, (vMin + vMax) / 2.0);
    double uLength = p1.Distance(p2);

    int lineCount = std::max(2, static_cast<int>(uLength / m_params.stepOver));
    int pointsPerLine = m_params.pointDensity;

    for (int i = 0; i < lineCount; ++i) {
        double u = uMin + (uMax - uMin) * i / (lineCount - 1);

        for (int j = 0; j < pointsPerLine; ++j) {
            double v = vMin + (vMax - vMin) * j / (pointsPerLine - 1);

            BRepLProp_SLProps props(adaptor, u, v, 2, Precision::Confusion());
            gp_Pnt pt = adaptor.Value(u, v);

            PathPoint pp;
            pp.position = QVector3D(
                static_cast<float>(pt.X()),
                static_cast<float>(pt.Y()),
                static_cast<float>(pt.Z()));

            if (props.IsNormalDefined()) {
                gp_Dir normal = props.Normal();
                if (m_face.Orientation() == TopAbs_REVERSED) {
                    normal.Reverse();
                }
                pp.normal = QVector3D(
                    static_cast<float>(normal.X()),
                    static_cast<float>(normal.Y()),
                    static_cast<float>(normal.Z()));
            }

            pp.feedRate = m_params.feedRate;
            pp.pressure = m_params.pressure;

            path.append(pp);
        }
    }

    return path;
}

// ============================================================================
// 之字形路径（每行方向交替）
// ============================================================================
QVector<PathPoint> GrindingPathGenerator::generateZigzag()
{
    QVector<PathPoint> path;

    BRepAdaptor_Surface adaptor(m_face);
    double uMin = adaptor.FirstUParameter();
    double uMax = adaptor.LastUParameter();
    double vMin = adaptor.FirstVParameter();
    double vMax = adaptor.LastVParameter();

    gp_Pnt p1 = adaptor.Value(uMin, (vMin + vMax) / 2.0);
    gp_Pnt p2 = adaptor.Value(uMax, (vMin + vMax) / 2.0);
    double uLength = p1.Distance(p2);

    int lineCount = std::max(2, static_cast<int>(uLength / m_params.stepOver));
    int pointsPerLine = m_params.pointDensity;

    for (int i = 0; i < lineCount; ++i) {
        double u = uMin + (uMax - uMin) * i / (lineCount - 1);
        bool reverse = (i % 2 == 1);  // 交替方向

        for (int jj = 0; jj < pointsPerLine; ++jj) {
            int j = reverse ? (pointsPerLine - 1 - jj) : jj;
            double v = vMin + (vMax - vMin) * j / (pointsPerLine - 1);

            BRepLProp_SLProps props(adaptor, u, v, 2, Precision::Confusion());
            gp_Pnt pt = adaptor.Value(u, v);

            PathPoint pp;
            pp.position = QVector3D(
                static_cast<float>(pt.X()),
                static_cast<float>(pt.Y()),
                static_cast<float>(pt.Z()));

            if (props.IsNormalDefined()) {
                gp_Dir normal = props.Normal();
                if (m_face.Orientation() == TopAbs_REVERSED) {
                    normal.Reverse();
                }
                pp.normal = QVector3D(
                    static_cast<float>(normal.X()),
                    static_cast<float>(normal.Y()),
                    static_cast<float>(normal.Z()));
            }

            pp.feedRate = m_params.feedRate;
            pp.pressure = m_params.pressure;
            path.append(pp);
        }
    }

    return path;
}

// ============================================================================
// 螺旋路径
// ============================================================================
QVector<PathPoint> GrindingPathGenerator::generateSpiral()
{
    QVector<PathPoint> path;

    BRepAdaptor_Surface adaptor(m_face);
    double uMin = adaptor.FirstUParameter();
    double uMax = adaptor.LastUParameter();
    double vMin = adaptor.FirstVParameter();
    double vMax = adaptor.LastVParameter();

    double uCenter = (uMin + uMax) / 2.0;
    double vCenter = (vMin + vMax) / 2.0;
    double uRange = (uMax - uMin) / 2.0;
    double vRange = (vMax - vMin) / 2.0;

    // 估算圈数
    gp_Pnt pCenter = adaptor.Value(uCenter, vCenter);
    gp_Pnt pEdge = adaptor.Value(uMax, vCenter);
    double radius = pCenter.Distance(pEdge);
    int turns = std::max(2, static_cast<int>(radius / m_params.stepOver));

    int totalPoints = turns * m_params.pointDensity;

    for (int i = 0; i < totalPoints; ++i) {
        double t = static_cast<double>(i) / totalPoints;
        double angle = t * turns * 2.0 * M_PI;
        double r = t;  // 归一化半径 0->1

        double u = uCenter + uRange * r * qCos(angle);
        double v = vCenter + vRange * r * qSin(angle);

        // 裁剪到参数域内
        u = qBound(uMin, u, uMax);
        v = qBound(vMin, v, vMax);

        BRepLProp_SLProps props(adaptor, u, v, 2, Precision::Confusion());
        gp_Pnt pt = adaptor.Value(u, v);

        PathPoint pp;
        pp.position = QVector3D(
            static_cast<float>(pt.X()),
            static_cast<float>(pt.Y()),
            static_cast<float>(pt.Z()));

        if (props.IsNormalDefined()) {
            gp_Dir normal = props.Normal();
            if (m_face.Orientation() == TopAbs_REVERSED) {
                normal.Reverse();
            }
            pp.normal = QVector3D(
                static_cast<float>(normal.X()),
                static_cast<float>(normal.Y()),
                static_cast<float>(normal.Z()));
        }

        pp.feedRate = m_params.feedRate;
        pp.pressure = m_params.pressure;
        path.append(pp);
    }

    return path;
}

// ============================================================================
// 等高线路径
// ============================================================================
QVector<PathPoint> GrindingPathGenerator::generateContour()
{
    QVector<PathPoint> path;

    BRepAdaptor_Surface adaptor(m_face);
    double uMin = adaptor.FirstUParameter();
    double uMax = adaptor.LastUParameter();
    double vMin = adaptor.FirstVParameter();
    double vMax = adaptor.LastVParameter();

    // 沿 V 方向的等参数线（等高线）
    gp_Pnt p1 = adaptor.Value((uMin + uMax) / 2.0, vMin);
    gp_Pnt p2 = adaptor.Value((uMin + uMax) / 2.0, vMax);
    double vLength = p1.Distance(p2);

    int lineCount = std::max(2, static_cast<int>(vLength / m_params.stepOver));
    int pointsPerLine = m_params.pointDensity;

    for (int i = 0; i < lineCount; ++i) {
        double v = vMin + (vMax - vMin) * i / (lineCount - 1);

        for (int j = 0; j < pointsPerLine; ++j) {
            double u = uMin + (uMax - uMin) * j / (pointsPerLine - 1);

            BRepLProp_SLProps props(adaptor, u, v, 2, Precision::Confusion());
            gp_Pnt pt = adaptor.Value(u, v);

            PathPoint pp;
            pp.position = QVector3D(
                static_cast<float>(pt.X()),
                static_cast<float>(pt.Y()),
                static_cast<float>(pt.Z()));

            if (props.IsNormalDefined()) {
                gp_Dir normal = props.Normal();
                if (m_face.Orientation() == TopAbs_REVERSED) {
                    normal.Reverse();
                }
                pp.normal = QVector3D(
                    static_cast<float>(normal.X()),
                    static_cast<float>(normal.Y()),
                    static_cast<float>(normal.Z()));
            }

            pp.feedRate = m_params.feedRate;
            pp.pressure = m_params.pressure;
            path.append(pp);
        }
    }

    return path;
}

// ============================================================================
// 路径平滑 - 三次移动平均
// ============================================================================
QVector<PathPoint> GrindingPathGenerator::smoothPath(const QVector<PathPoint>& path)
{
    if (path.size() < 5) return path;

    QVector<PathPoint> smoothed = path;
    const int windowSize = 5;
    const int halfWindow = windowSize / 2;

    for (int i = halfWindow; i < path.size() - halfWindow; ++i) {
        float sx = 0, sy = 0, sz = 0;
        float nx = 0, ny = 0, nz = 0;

        for (int w = -halfWindow; w <= halfWindow; ++w) {
            const auto& pt = path[i + w];
            sx += pt.position.x();
            sy += pt.position.y();
            sz += pt.position.z();
            nx += pt.normal.x();
            ny += pt.normal.y();
            nz += pt.normal.z();
        }

        float div = static_cast<float>(windowSize);
        smoothed[i].position = QVector3D(sx / div, sy / div, sz / div);

        QVector3D avgNormal(nx / div, ny / div, nz / div);
        if (avgNormal.length() > 0.001f) {
            avgNormal.normalize();
        }
        smoothed[i].normal = avgNormal;
    }

    return smoothed;
}

// ============================================================================
// 进退刀路径
// ============================================================================
QVector<PathPoint> GrindingPathGenerator::addApproachRetract(
    const QVector<PathPoint>& path)
{
    if (path.isEmpty()) return path;

    QVector<PathPoint> result;
    double safeH = m_params.safeHeight;

    // 进刀点：第一个路径点沿法向偏移安全高度
    PathPoint approach = path.first();
    approach.position += approach.normal * static_cast<float>(safeH);
    approach.feedRate = m_params.feedRate * 0.5;  // 进刀速度减半
    result.append(approach);

    // 主路径
    result.append(path);

    // 退刀点：最后一个路径点沿法向偏移安全高度
    PathPoint retract = path.last();
    retract.position += retract.normal * static_cast<float>(safeH);
    retract.feedRate = m_params.feedRate * 0.5;
    result.append(retract);

    return result;
}

double GrindingPathGenerator::distance(const PathPoint& a, const PathPoint& b)
{
    QVector3D diff = a.position - b.position;
    return static_cast<double>(diff.length());
}
