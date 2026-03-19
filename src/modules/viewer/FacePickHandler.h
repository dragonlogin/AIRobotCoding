#pragma once

#include <osgGA/GUIEventHandler>
#include <osgViewer/Viewer>

/**
 * @brief Face pick event handler
 *
 * Implements face selection in a 3D scene via ray-intersection testing.
 * Supports single-select, multi-select (Ctrl), and highlight feedback.
 */
class FacePickHandler : public osgGA::GUIEventHandler
{
public:
    FacePickHandler(osgViewer::Viewer* viewer);

    bool handle(const osgGA::GUIEventAdapter& ea,
                osgGA::GUIActionAdapter& aa) override;

private:
    /// Perform ray-based picking
    void pick(float x, float y, bool multiSelect);

    /// Highlight the specified face
    void highlightFace(osg::Node* node, bool highlight);

    /// Clear all highlights
    void clearHighlights();

    osgViewer::Viewer* m_viewer = nullptr;
    osg::ref_ptr<osg::Node> m_lastPicked;
    std::vector<osg::ref_ptr<osg::Node>> m_selectedNodes;
};
