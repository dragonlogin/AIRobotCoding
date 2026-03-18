#include "StepReader.h"

// OCC STEP 读取
#include <STEPControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>

// OCC 拓扑遍历
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Solid.hxx>

// OCC 几何分析
#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepLProp_SLProps.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <Geom_Surface.hxx>
#include <GeomLProp_SLProps.hxx>
#include <ShapeAnalysis_Surface.hxx>

// OCC 网格化（供后续 OSG 使用）
#include <BRepMesh_IncrementalMesh.hxx>

bool StepReader::load(const QString& filePath)
{
    STEPControl_Reader reader;

    // 读取文件
    IFSelect_ReturnStatus status = reader.ReadFile(filePath.toUtf8().constData());
    if (status != IFSelect_RetDone) {
        m_errorString = QString("无法读取 STEP 文件: %1 (错误码: %2)")
                            .arg(filePath)
                            .arg(static_cast<int>(status));
        return false;
    }

    // 转换所有根实体
    Standard_Integer nbRoots = reader.NbRootsForTransfer();
    if (nbRoots == 0) {
        m_errorString = "STEP 文件中没有可转换的实体";
        return false;
    }

    reader.TransferRoots();
    m_shape = reader.OneShape();

    if (m_shape.IsNull()) {
        m_errorString = "转换后的形状为空";
        return false;
    }

    // 预先进行网格化（三角剖分），精度 0.1mm
    BRepMesh_IncrementalMesh mesh(m_shape, 0.1);
    mesh.Perform();

    if (!mesh.IsDone()) {
        m_errorString = "网格化失败";
        return false;
    }

    return true;
}

QVector<SurfaceInfo> StepReader::analyzeSurfaces() const
{
    QVector<SurfaceInfo> surfaces;

    if (m_shape.IsNull())
        return surfaces;

    int faceIndex = 0;
    for (TopExp_Explorer explorer(m_shape, TopAbs_FACE); explorer.More(); explorer.Next()) {
        const TopoDS_Face& face = TopoDS::Face(explorer.Current());
        SurfaceInfo info;
        info.faceIndex = faceIndex++;

        // === 曲面类型 ===
        BRepAdaptor_Surface adaptor(face);
        GeomAbs_SurfaceType surfType = adaptor.GetType();

        switch (surfType) {
        case GeomAbs_Plane:
            info.surfaceType = "Plane";
            break;
        case GeomAbs_Cylinder:
            info.surfaceType = "Cylinder";
            break;
        case GeomAbs_Cone:
            info.surfaceType = "Cone";
            break;
        case GeomAbs_Sphere:
            info.surfaceType = "Sphere";
            break;
        case GeomAbs_Torus:
            info.surfaceType = "Torus";
            break;
        case GeomAbs_BezierSurface:
            info.surfaceType = "Bezier";
            break;
        case GeomAbs_BSplineSurface:
            info.surfaceType = "BSpline";
            break;
        case GeomAbs_SurfaceOfRevolution:
            info.surfaceType = "Revolution";
            break;
        case GeomAbs_SurfaceOfExtrusion:
            info.surfaceType = "Extrusion";
            break;
        case GeomAbs_OffsetSurface:
            info.surfaceType = "Offset";
            break;
        default:
            info.surfaceType = "Other";
            break;
        }

        // === 面积 ===
        GProp_GProps props;
        BRepGProp::SurfaceProperties(face, props);
        info.area = props.Mass();  // 面积值

        // === 曲率分析 ===
        // 在面的中心点处计算曲率和法向
        double uMin = adaptor.FirstUParameter();
        double uMax = adaptor.LastUParameter();
        double vMin = adaptor.FirstVParameter();
        double vMax = adaptor.LastVParameter();
        double uMid = (uMin + uMax) / 2.0;
        double vMid = (vMin + vMax) / 2.0;

        BRepLProp_SLProps slProps(adaptor, uMid, vMid, 2, Precision::Confusion());

        if (slProps.IsNormalDefined()) {
            gp_Dir normal = slProps.Normal();
            // 考虑面的朝向
            if (face.Orientation() == TopAbs_REVERSED) {
                normal.Reverse();
            }
            info.normal = QVector3D(
                static_cast<float>(normal.X()),
                static_cast<float>(normal.Y()),
                static_cast<float>(normal.Z()));
        }

        if (slProps.IsCurvatureDefined()) {
            info.minCurvature = slProps.MinCurvature();
            info.maxCurvature = slProps.MaxCurvature();
        }

        // === 多点采样获取更精确的曲率范围 ===
        double globalMinCurv = 1e10;
        double globalMaxCurv = -1e10;
        const int sampleCount = 5;

        for (int i = 0; i < sampleCount; ++i) {
            for (int j = 0; j < sampleCount; ++j) {
                double u = uMin + (uMax - uMin) * (i + 0.5) / sampleCount;
                double v = vMin + (vMax - vMin) * (j + 0.5) / sampleCount;

                BRepLProp_SLProps sampleProps(adaptor, u, v, 2, Precision::Confusion());
                if (sampleProps.IsCurvatureDefined()) {
                    double minC = sampleProps.MinCurvature();
                    double maxC = sampleProps.MaxCurvature();
                    if (minC < globalMinCurv) globalMinCurv = minC;
                    if (maxC > globalMaxCurv) globalMaxCurv = maxC;
                }
            }
        }

        if (globalMinCurv < 1e10) {
            info.minCurvature = globalMinCurv;
            info.maxCurvature = globalMaxCurv;
        }

        surfaces.append(info);
    }

    return surfaces;
}
