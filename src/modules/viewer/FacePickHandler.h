#pragma once

#include <osgGA/GUIEventHandler>
#include <osgViewer/Viewer>

/**
 * @brief 面拾取事件处理器
 *
 * 通过射线相交检测实现3D场景中的面选择，
 * 支持单选、多选（Ctrl）和高亮反馈。
 */
class FacePickHandler : public osgGA::GUIEventHandler
{
public:
    FacePickHandler(osgViewer::Viewer* viewer);

    bool handle(const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& aa) override;

private:
    /// 执行射线拾取
    void pick(float x, float y, bool multiSelect);

    /// 高亮指定面
    void highlightFace(osg::Node* node, bool highlight);

    /// 清除所有高亮
    void clearHighlights();

    osgViewer::Viewer* m_viewer = nullptr;
    osg::ref_ptr<osg::Node> m_lastPicked;
    std::vector<osg::ref_ptr<osg::Node>> m_selectedNodes;
};
