#include "OsgViewerWidget.h"
#include "core/EventBus.h"

#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/LineWidth>
#include <osg/Light>
#include <osg/LightSource>
#include <osgGA/TrackballManipulator>

#include <QMouseEvent>
#include <QWheelEvent>

OsgViewerWidget::OsgViewerWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setMinimumSize(400, 300);
    setFocusPolicy(Qt::StrongFocus);

    // 创建场景层级
    m_sceneRoot = new osg::Group;
    m_modelGroup = new osg::Group;
    m_robotGroup = new osg::Group;
    m_pathGroup = new osg::Group;
    m_helperGroup = new osg::Group;

    m_sceneRoot->addChild(m_modelGroup);
    m_sceneRoot->addChild(m_robotGroup);
    m_sceneRoot->addChild(m_pathGroup);
    m_sceneRoot->addChild(m_helperGroup);

    m_modelGroup->setName("Models");
    m_robotGroup->setName("Robot");
    m_pathGroup->setName("Paths");
    m_helperGroup->setName("Helpers");

    // 渲染定时器 60fps
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, QOverload<>::of(&QOpenGLWidget::update));
    m_updateTimer->start(16);
}

OsgViewerWidget::~OsgViewerWidget()
{
    m_updateTimer->stop();
}

void OsgViewerWidget::initializeGL()
{
    // 创建嵌入式图形窗口
    m_graphicsWindow = new osgViewer::GraphicsWindowEmbedded(
        x(), y(), width(), height());

    m_viewer = new osgViewer::Viewer;
    m_viewer->setSceneData(m_sceneRoot);

    setupCamera();
    setupLighting();
    setupScene();
}

void OsgViewerWidget::paintGL()
{
    if (m_viewer.valid()) {
        m_viewer->frame();
    }
}

void OsgViewerWidget::resizeGL(int w, int h)
{
    if (m_graphicsWindow.valid()) {
        m_graphicsWindow->resized(x(), y(), w, h);
        m_graphicsWindow->getEventQueue()->windowResize(x(), y(), w, h);
    }
}

void OsgViewerWidget::setupCamera()
{
    osg::Camera* camera = m_viewer->getCamera();
    camera->setGraphicsContext(m_graphicsWindow);
    camera->setViewport(0, 0, width(), height());
    camera->setClearColor(osg::Vec4(0.17f, 0.17f, 0.17f, 1.0f)); // 深灰背景
    camera->setProjectionMatrixAsPerspective(
        45.0, static_cast<double>(width()) / height(), 0.1, 10000.0);

    m_viewer->setCameraManipulator(new osgGA::TrackballManipulator);
}

void OsgViewerWidget::setupLighting()
{
    osg::ref_ptr<osg::Light> light = new osg::Light;
    light->setLightNum(0);
    light->setPosition(osg::Vec4(500, 500, 1000, 1.0));
    light->setAmbient(osg::Vec4(0.3f, 0.3f, 0.3f, 1.0f));
    light->setDiffuse(osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f));
    light->setSpecular(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

    osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource;
    lightSource->setLight(light);
    m_sceneRoot->addChild(lightSource);
}

void OsgViewerWidget::setupScene()
{
    addCoordinateAxes();
    addGridPlane();
}

void OsgViewerWidget::addCoordinateAxes()
{
    osg::ref_ptr<osg::Geode> axisGeode = new osg::Geode;
    axisGeode->setName("CoordinateAxes");
    m_helperGroup->addChild(axisGeode);
}

void OsgViewerWidget::addGridPlane()
{
    osg::ref_ptr<osg::Geode> gridGeode = new osg::Geode;
    gridGeode->setName("GridPlane");
    m_helperGroup->addChild(gridGeode);
}

void OsgViewerWidget::addSceneNode(osg::Node* node, const QString& name)
{
    if (!node) return;
    node->setName(name.toStdString());
    m_modelGroup->addChild(node);
    m_namedNodes.insert(name, node);
}

void OsgViewerWidget::removeSceneNode(const QString& name)
{
    auto it = m_namedNodes.find(name);
    if (it != m_namedNodes.end()) {
        m_modelGroup->removeChild(it.value());
        m_namedNodes.erase(it);
    }
}

// ============================================================================
// 鼠标事件转发给 OSG
// ============================================================================
void OsgViewerWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_graphicsWindow.valid()) {
        m_graphicsWindow->getEventQueue()->mouseButtonPress(
            event->x(), event->y(),
            event->button() == Qt::LeftButton ? 1 :
            event->button() == Qt::MiddleButton ? 2 : 3);
    }
}

void OsgViewerWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_graphicsWindow.valid()) {
        m_graphicsWindow->getEventQueue()->mouseButtonRelease(
            event->x(), event->y(),
            event->button() == Qt::LeftButton ? 1 :
            event->button() == Qt::MiddleButton ? 2 : 3);
    }
}

void OsgViewerWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_graphicsWindow.valid()) {
        m_graphicsWindow->getEventQueue()->mouseMotion(event->x(), event->y());
    }
}

void OsgViewerWidget::wheelEvent(QWheelEvent* event)
{
    if (m_graphicsWindow.valid()) {
        m_graphicsWindow->getEventQueue()->mouseScroll(
            event->angleDelta().y() > 0 ?
                osgGA::GUIEventAdapter::SCROLL_UP :
                osgGA::GUIEventAdapter::SCROLL_DOWN);
    }
}

// ============================================================================
// 视图控制
// ============================================================================
void OsgViewerWidget::setViewFront()
{
    auto* manip = dynamic_cast<osgGA::TrackballManipulator*>(
        m_viewer->getCameraManipulator());
    if (manip) {
        manip->setRotation(osg::Quat(0, 0, 0, 1));
    }
}

void OsgViewerWidget::setViewIsometric()
{
    auto* manip = dynamic_cast<osgGA::TrackballManipulator*>(
        m_viewer->getCameraManipulator());
    if (manip) {
        osg::Quat rot;
        rot.makeRotate(
            osg::DegreesToRadians(35.264), osg::Vec3(1, 0, 0),
            osg::DegreesToRadians(0.0),    osg::Vec3(0, 1, 0),
            osg::DegreesToRadians(45.0),   osg::Vec3(0, 0, 1));
        manip->setRotation(rot);
    }
}

void OsgViewerWidget::fitAll()
{
    if (m_viewer.valid()) {
        m_viewer->getCameraManipulator()->home(0);
    }
}

void OsgViewerWidget::setViewBack() { /* 类似 setViewFront, 旋转180° */ }
void OsgViewerWidget::setViewLeft() { /* Y轴旋转-90° */ }
void OsgViewerWidget::setViewRight() { /* Y轴旋转90° */ }
void OsgViewerWidget::setViewTop() { /* X轴旋转-90° */ }
void OsgViewerWidget::setViewBottom() { /* X轴旋转90° */ }
