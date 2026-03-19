#pragma once

#include <QOpenGLWidget>
#include <QTimer>
#include <QMap>

#include <osgViewer/Viewer>
#include <osgViewer/GraphicsWindow>
#include <osg/Group>
#include <osg/MatrixTransform>

/**
 * @brief OSG 3D Viewer Widget - OpenSceneGraph renderer embedded in Qt
 *
 * Responsibilities:
 * - 3D scene rendering (workpiece model, robot, path, coordinate axes)
 * - Interactive operations (rotate, pan, zoom, pick)
 * - Scene node management interface for use by other modules
 */
class OsgViewerWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit OsgViewerWidget(QWidget* parent = nullptr);
    ~OsgViewerWidget();

    /// Returns the scene root node (modules attach content to this node)
    osg::Group* sceneRoot() const { return m_sceneRoot.get(); }

    /// Add/remove a named scene node
    void addSceneNode(osg::Node* node, const QString& name);
    void removeSceneNode(const QString& name);

    /// View controls
    void setViewFront();
    void setViewBack();
    void setViewLeft();
    void setViewRight();
    void setViewTop();
    void setViewBottom();
    void setViewIsometric();
    void fitAll();

signals:
    void cursorMoved(double x, double y, double z);
    void objectPicked(int faceIndex);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void setupScene();
    void setupCamera();
    void setupLighting();
    void addCoordinateAxes();
    void addGridPlane();

    osg::ref_ptr<osgViewer::Viewer> m_viewer;
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> m_graphicsWindow;
    osg::ref_ptr<osg::Group> m_sceneRoot;
    osg::ref_ptr<osg::Group> m_modelGroup;       // workpiece model
    osg::ref_ptr<osg::Group> m_robotGroup;        // robot model
    osg::ref_ptr<osg::Group> m_pathGroup;         // grinding path
    osg::ref_ptr<osg::Group> m_helperGroup;       // helper elements (grid, coordinate axes)

    QTimer* m_updateTimer = nullptr;

    // Named node map
    QMap<QString, osg::ref_ptr<osg::Node>> m_namedNodes;
};
