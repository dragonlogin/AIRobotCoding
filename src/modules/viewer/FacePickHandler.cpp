#include "FacePickHandler.h"
#include "core/EventBus.h"

#include <osgUtil/LineSegmentIntersector>
#include <osg/Geode>
#include <osg/Material>
#include <osg/StateSet>

FacePickHandler::FacePickHandler(osgViewer::Viewer* viewer)
    : m_viewer(viewer)
{
}

bool FacePickHandler::handle(
    const osgGA::GUIEventAdapter& ea,
    osgGA::GUIActionAdapter& aa)
{
    if (ea.getEventType() != osgGA::GUIEventAdapter::RELEASE)
        return false;

    if (ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
        return false;

    // 检查是否有鼠标移动（拖拽不触发拾取）
    if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE) {
        bool multiSelect = (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL) != 0;
        pick(ea.getX(), ea.getY(), multiSelect);
    }

    return false;  // 不消费事件，让相机操纵器继续工作
}

void FacePickHandler::pick(float x, float y, bool multiSelect)
{
    if (!m_viewer) return;

    osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
        new osgUtil::LineSegmentIntersector(
            osgUtil::Intersector::WINDOW, x, y);

    osgUtil::IntersectionVisitor visitor(intersector);
    m_viewer->getCamera()->accept(visitor);

    if (!intersector->containsIntersections()) {
        if (!multiSelect) {
            clearHighlights();
        }
        return;
    }

    // 获取最近的交点
    auto& intersection = intersector->getFirstIntersection();
    osg::NodePath& nodePath = const_cast<osg::NodePath&>(intersection.nodePath);

    // 查找带有 FaceUserData 的 Geode 节点
    osg::Node* pickedNode = nullptr;
    int faceIndex = -1;

    for (auto it = nodePath.rbegin(); it != nodePath.rend(); ++it) {
        osg::Referenced* userData = (*it)->getUserData();
        if (userData) {
            // 通过 dynamic_cast 或名称约定获取 faceIndex
            // 这里通过节点名称解析
            std::string name = (*it)->getName();
            if (name.substr(0, 5) == "Face_") {
                faceIndex = std::stoi(name.substr(5));
                pickedNode = *it;
                break;
            }
        }
    }

    if (!pickedNode || faceIndex < 0) return;

    if (!multiSelect) {
        clearHighlights();
    }

    // 切换选择状态
    auto it = std::find(m_selectedNodes.begin(), m_selectedNodes.end(), pickedNode);
    if (it != m_selectedNodes.end()) {
        // 已选中，取消选择
        highlightFace(pickedNode, false);
        m_selectedNodes.erase(it);
    } else {
        // 新选中
        highlightFace(pickedNode, true);
        m_selectedNodes.push_back(pickedNode);
    }

    // 发布选择事件
    EventBus::instance()->publish("cad.face.selected", {
        {"faceIndex", faceIndex}
    });

    // 发布 3D 坐标
    osg::Vec3d worldPoint = intersection.getWorldIntersectPoint();
    EventBus::instance()->publish("viewer.cursor.moved", {
        {"x", worldPoint.x()},
        {"y", worldPoint.y()},
        {"z", worldPoint.z()}
    });
}

void FacePickHandler::highlightFace(osg::Node* node, bool highlight)
{
    if (!node) return;

    osg::StateSet* stateSet = node->getOrCreateStateSet();
    osg::Material* material = dynamic_cast<osg::Material*>(
        stateSet->getAttribute(osg::StateAttribute::MATERIAL));

    if (!material) {
        material = new osg::Material;
        stateSet->setAttributeAndModes(material, osg::StateAttribute::ON);
    }

    if (highlight) {
        material->setDiffuse(osg::Material::FRONT_AND_BACK,
            osg::Vec4(0.2f, 0.6f, 1.0f, 0.85f));
        material->setEmission(osg::Material::FRONT_AND_BACK,
            osg::Vec4(0.05f, 0.15f, 0.3f, 1.0f));
    } else {
        material->setDiffuse(osg::Material::FRONT_AND_BACK,
            osg::Vec4(0.7f, 0.75f, 0.8f, 1.0f));
        material->setEmission(osg::Material::FRONT_AND_BACK,
            osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
}

void FacePickHandler::clearHighlights()
{
    for (auto& node : m_selectedNodes) {
        highlightFace(node, false);
    }
    m_selectedNodes.clear();
}
