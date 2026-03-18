#include "OccToOsgConverter.h"

// OCC 拓扑/三角化
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <TopLoc_Location.hxx>
#include <gp_Pnt.hxx>
#include <TColgp_Array1OfPnt.hxx>

// OSG
#include <osg/Material>
#include <osg/StateSet>
#include <osg/BlendFunc>
#include <osg/CullFace>
#include <osgUtil/SmoothingVisitor>

/**
 * @brief 存储在 osg::Node 的 UserData 中，用于面拾取时识别 faceIndex
 */
class FaceUserData : public osg::Referenced
{
public:
    FaceUserData(int index) : faceIndex(index) {}
    int faceIndex;
};

osg::ref_ptr<osg::Group> OccToOsgConverter::convertShape(const TopoDS_Shape& shape)
{
    osg::ref_ptr<osg::Group> group = new osg::Group;
    group->setName("CADModel");

    if (shape.IsNull())
        return group;

    int faceIndex = 0;
    for (TopExp_Explorer explorer(shape, TopAbs_FACE); explorer.More(); explorer.Next()) {
        const TopoDS_Face& face = TopoDS::Face(explorer.Current());
        osg::ref_ptr<osg::Geode> geode = convertFace(face, faceIndex);
        if (geode.valid()) {
            group->addChild(geode);
        }
        ++faceIndex;
    }

    return group;
}

osg::ref_ptr<osg::Geode> OccToOsgConverter::convertFace(
    const TopoDS_Face& face, int faceIndex)
{
    // 获取三角剖分数据
    TopLoc_Location location;
    Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);

    if (triangulation.IsNull())
        return nullptr;

    const int nbNodes = triangulation->NbNodes();
    const int nbTriangles = triangulation->NbTriangles();

    if (nbNodes == 0 || nbTriangles == 0)
        return nullptr;

    // 创建 OSG 几何体
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;

    // === 顶点 ===
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(nbNodes);
    for (int i = 1; i <= nbNodes; ++i) {
        gp_Pnt pt = triangulation->Node(i);
        // 应用变换
        pt.Transform(location.Transformation());
        (*vertices)[i - 1].set(
            static_cast<float>(pt.X()),
            static_cast<float>(pt.Y()),
            static_cast<float>(pt.Z()));
    }
    geometry->setVertexArray(vertices);

    // === 法线 ===
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(nbNodes);
    if (triangulation->HasNormals()) {
        for (int i = 1; i <= nbNodes; ++i) {
            gp_Dir dir = triangulation->Normal(i);
            if (face.Orientation() == TopAbs_REVERSED) {
                dir.Reverse();
            }
            (*normals)[i - 1].set(
                static_cast<float>(dir.X()),
                static_cast<float>(dir.Y()),
                static_cast<float>(dir.Z()));
        }
    }
    geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);

    // === 三角面索引 ===
    osg::ref_ptr<osg::DrawElementsUInt> indices =
        new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, nbTriangles * 3);

    for (int i = 1; i <= nbTriangles; ++i) {
        int n1, n2, n3;
        triangulation->Triangle(i).Get(n1, n2, n3);

        // 考虑面朝向
        if (face.Orientation() == TopAbs_REVERSED) {
            std::swap(n2, n3);
        }

        (*indices)[(i - 1) * 3 + 0] = n1 - 1;
        (*indices)[(i - 1) * 3 + 1] = n2 - 1;
        (*indices)[(i - 1) * 3 + 2] = n3 - 1;
    }
    geometry->addPrimitiveSet(indices);

    // === 颜色 ===
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
    (*colors)[0].set(m_defaultColor[0], m_defaultColor[1],
                     m_defaultColor[2], m_defaultColor[3]);
    geometry->setColorArray(colors, osg::Array::BIND_OVERALL);

    // === 材质 ===
    osg::ref_ptr<osg::Material> material = new osg::Material;
    material->setDiffuse(osg::Material::FRONT_AND_BACK,
        osg::Vec4(m_defaultColor[0], m_defaultColor[1],
                  m_defaultColor[2], m_defaultColor[3]));
    material->setSpecular(osg::Material::FRONT_AND_BACK,
        osg::Vec4(0.3f, 0.3f, 0.3f, 1.0f));
    material->setShininess(osg::Material::FRONT_AND_BACK, 30.0f);
    material->setAmbient(osg::Material::FRONT_AND_BACK,
        osg::Vec4(0.15f, 0.15f, 0.18f, 1.0f));

    // === Geode ===
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(geometry);
    geode->setName(std::string("Face_") + std::to_string(faceIndex));

    // 存储 faceIndex 到 UserData，用于拾取
    geode->setUserData(new FaceUserData(faceIndex));

    // 状态设置
    osg::StateSet* stateSet = geode->getOrCreateStateSet();
    stateSet->setAttributeAndModes(material, osg::StateAttribute::ON);
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    // 背面剔除
    osg::ref_ptr<osg::CullFace> cullFace = new osg::CullFace(osg::CullFace::BACK);
    stateSet->setAttributeAndModes(cullFace, osg::StateAttribute::ON);

    return geode;
}

void OccToOsgConverter::setDefaultColor(float r, float g, float b, float a)
{
    m_defaultColor[0] = r;
    m_defaultColor[1] = g;
    m_defaultColor[2] = b;
    m_defaultColor[3] = a;
}

void OccToOsgConverter::setHighlightColor(float r, float g, float b, float a)
{
    m_highlightColor[0] = r;
    m_highlightColor[1] = g;
    m_highlightColor[2] = b;
    m_highlightColor[3] = a;
}
