#include "PathVisualizer.h"

#include <osg/LineWidth>
#include <osg/Point>
#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>

osg::ref_ptr<osg::Group> PathVisualizer::createPathNode(
    const QVector<PathPoint>& path)
{
    osg::ref_ptr<osg::Group> group = new osg::Group;
    group->setName("GrindingPath");

    if (path.isEmpty()) return group;

    // === 路径线条 ===
    osg::ref_ptr<osg::Geometry> lineGeom = new osg::Geometry;

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(path.size());
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(path.size());

    for (int i = 0; i < path.size(); ++i) {
        const auto& pt = path[i];
        (*vertices)[i].set(pt.position.x(), pt.position.y(), pt.position.z());

        // 颜色渐变：绿色(起点) -> 黄色(中点) -> 红色(终点)
        float t = static_cast<float>(i) / std::max(1, path.size() - 1);
        if (t < 0.5f) {
            float s = t * 2.0f;
            (*colors)[i].set(s, 1.0f, 0.0f, 1.0f);  // 绿 -> 黄
        } else {
            float s = (t - 0.5f) * 2.0f;
            (*colors)[i].set(1.0f, 1.0f - s, 0.0f, 1.0f);  // 黄 -> 红
        }
    }

    lineGeom->setVertexArray(vertices);
    lineGeom->setColorArray(colors, osg::Array::BIND_PER_VERTEX);
    lineGeom->addPrimitiveSet(
        new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, path.size()));

    // 线宽
    osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth(m_lineWidth);
    lineGeom->getOrCreateStateSet()->setAttributeAndModes(
        lineWidth, osg::StateAttribute::ON);
    lineGeom->getOrCreateStateSet()->setMode(
        GL_LIGHTING, osg::StateAttribute::OFF);

    osg::ref_ptr<osg::Geode> lineGeode = new osg::Geode;
    lineGeode->addDrawable(lineGeom);
    lineGeode->setName("PathLine");
    group->addChild(lineGeode);

    // === 路径点 ===
    osg::ref_ptr<osg::Geometry> pointGeom = new osg::Geometry;
    pointGeom->setVertexArray(vertices);

    osg::ref_ptr<osg::Vec4Array> pointColors = new osg::Vec4Array(1);
    (*pointColors)[0].set(1.0f, 1.0f, 1.0f, 1.0f);
    pointGeom->setColorArray(pointColors, osg::Array::BIND_OVERALL);

    pointGeom->addPrimitiveSet(
        new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, path.size()));

    osg::ref_ptr<osg::Point> point = new osg::Point(4.0f);
    pointGeom->getOrCreateStateSet()->setAttributeAndModes(
        point, osg::StateAttribute::ON);
    pointGeom->getOrCreateStateSet()->setMode(
        GL_LIGHTING, osg::StateAttribute::OFF);

    osg::ref_ptr<osg::Geode> pointGeode = new osg::Geode;
    pointGeode->addDrawable(pointGeom);
    pointGeode->setName("PathPoints");
    group->addChild(pointGeode);

    // === 法向箭头 ===
    osg::ref_ptr<osg::Geode> arrows = createNormalArrows(path);
    group->addChild(arrows);

    return group;
}

osg::ref_ptr<osg::Geode> PathVisualizer::createNormalArrows(
    const QVector<PathPoint>& path, float arrowLength)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->setName("NormalArrows");

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

    // 每隔几个点画一个法向箭头，避免过于密集
    int step = std::max(1, path.size() / 50);
    int arrowCount = 0;
    for (int i = 0; i < path.size(); i += step)
        ++arrowCount;

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(arrowCount * 2);
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
    (*colors)[0].set(0.3f, 0.8f, 1.0f, 0.7f);  // 浅蓝色

    int idx = 0;
    for (int i = 0; i < path.size(); i += step) {
        const auto& pt = path[i];
        osg::Vec3 pos(pt.position.x(), pt.position.y(), pt.position.z());
        osg::Vec3 norm(pt.normal.x(), pt.normal.y(), pt.normal.z());
        norm.normalize();

        (*vertices)[idx * 2 + 0] = pos;
        (*vertices)[idx * 2 + 1] = pos + norm * arrowLength;
        ++idx;
    }

    geom->setVertexArray(vertices);
    geom->setColorArray(colors, osg::Array::BIND_OVERALL);
    geom->addPrimitiveSet(
        new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, arrowCount * 2));

    osg::ref_ptr<osg::LineWidth> lw = new osg::LineWidth(1.5f);
    geom->getOrCreateStateSet()->setAttributeAndModes(lw, osg::StateAttribute::ON);
    geom->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    geode->addDrawable(geom);
    return geode;
}

osg::ref_ptr<osg::Geode> PathVisualizer::createCurrentPointMarker()
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->setName("CurrentPointMarker");

    osg::ref_ptr<osg::ShapeDrawable> sphere =
        new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0, 0, 0), 2.0f));
    sphere->setColor(osg::Vec4(1.0f, 0.3f, 0.1f, 1.0f));  // 红色

    geode->addDrawable(sphere);
    return geode;
}

void PathVisualizer::updateCurrentPoint(osg::Geode* marker, const PathPoint& point)
{
    if (!marker || marker->getNumParents() == 0) return;

    osg::MatrixTransform* transform =
        dynamic_cast<osg::MatrixTransform*>(marker->getParent(0));
    if (transform) {
        transform->setMatrix(osg::Matrix::translate(
            point.position.x(), point.position.y(), point.position.z()));
    }
}
