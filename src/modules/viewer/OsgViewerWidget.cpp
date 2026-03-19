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
#include <QOpenGLContext>
#include <QOpenGLFunctions>

OsgViewerWidget::OsgViewerWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setMinimumSize(400, 300);
    setFocusPolicy(Qt::StrongFocus);

    // Build scene hierarchy
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

    // Render timer at 60 fps
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
    // Create the embedded graphics window using physical pixels (2x logical on Retina)
    const double dpr = devicePixelRatio();
    m_graphicsWindow = new osgViewer::GraphicsWindowEmbedded(
        0, 0,
        static_cast<int>(width()  * dpr),
        static_cast<int>(height() * dpr));

    m_viewer = new osgViewer::Viewer;

    // Critical: single-threaded mode is required when embedding in QOpenGLWidget;
    // otherwise OSG spawns its own render thread, conflicts with the Qt GL context,
    // and causes State::apply() invalid operation errors and a blank viewport.
    m_viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);

    m_viewer->setSceneData(m_sceneRoot);

    setupCamera();

    // Fix "invalid operation" warnings in Geometry::drawImplementation():
    // When embedded in QOpenGLWidget under GL2 Compatibility Profile, OSG's
    // vertex-attribute aliasing and MVP uniforms conflict with the fixed pipeline.
    osg::State* glState = m_viewer->getCamera()->getGraphicsContext()->getState();
    glState->setUseModelViewAndProjectionUniforms(false);
    glState->setUseVertexAttributeAliasing(false);
    // In embedded mode Qt owns the GL context; OSG cannot track the full GL state
    // machine, so its per-attribute glGetError() checks produce spurious
    // "invalid operation" spam. Disable them entirely — real rendering errors
    // will still surface as visual artefacts.
    glState->setCheckForGLErrors(osg::State::NEVER_CHECK_GL_ERRORS);

    setupLighting();
    setupScene();

    // Qt's own GL initialization (glViewport etc.) may leave pending errors in the
    // error queue on some drivers. OSG's State::apply() calls glGetError() at the
    // very start and misreports those as its own "invalid operation". Drain the
    // queue here so the first frame starts with a clean error state.
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    while (f->glGetError() != GL_NO_ERROR) {}
}

void OsgViewerWidget::paintGL()
{
    if (!m_viewer.valid()) return;
    makeCurrent();
    // QOpenGLWidget renders into a Qt-managed FBO (not framebuffer 0).
    // OSG must be told the current FBO id every frame, otherwise it binds
    // the wrong framebuffer and generates "invalid operation" on every
    // State::apply() call.
    m_graphicsWindow->setDefaultFboId(defaultFramebufferObject());
    m_viewer->frame();
}

void OsgViewerWidget::resizeGL(int w, int h)
{
    if (!m_graphicsWindow.valid()) return;

    const double dpr = devicePixelRatio();
    const int pw = static_cast<int>(w * dpr);
    const int ph = static_cast<int>(h * dpr);

    m_graphicsWindow->resized(0, 0, pw, ph);
    m_graphicsWindow->getEventQueue()->windowResize(0, 0, pw, ph);

    if (m_viewer.valid()) {
        m_viewer->getCamera()->setViewport(0, 0, pw, ph);
        if (ph > 0) {
            m_viewer->getCamera()->setProjectionMatrixAsPerspective(
                45.0, static_cast<double>(pw) / ph, 0.1, 10000.0);
        }
    }
}

void OsgViewerWidget::setupCamera()
{
    // macOS Retina: use physical pixels, otherwise only the bottom-left quadrant is rendered
    const double dpr = devicePixelRatio();
    const int pw = static_cast<int>(width()  * dpr);
    const int ph = static_cast<int>(height() * dpr);

    osg::Camera* camera = m_viewer->getCamera();
    camera->setGraphicsContext(m_graphicsWindow);
    camera->setViewport(0, 0, pw, ph);
    camera->setClearColor(osg::Vec4(0.17f, 0.17f, 0.17f, 1.0f));
    camera->setProjectionMatrixAsPerspective(
        45.0, static_cast<double>(pw) / ph, 0.1, 10000.0);

    m_viewer->setCameraManipulator(new osgGA::TrackballManipulator);
}

void OsgViewerWidget::setupLighting()
{
    // Explicitly enable GL_LIGHTING on the root StateSet so that OSG's
    // Material attribute application doesn't produce "invalid enumerant" errors
    // on drivers that require lighting to be active before glMaterialfv calls.
    osg::StateSet* ss = m_sceneRoot->getOrCreateStateSet();
    ss->setMode(GL_LIGHTING, osg::StateAttribute::ON);

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
// Forward mouse events to OSG
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
// View controls
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

void OsgViewerWidget::setBackgroundColor(float r, float g, float b)
{
    if (m_viewer.valid())
        m_viewer->getCamera()->setClearColor(osg::Vec4(r, g, b, 1.0f));
}

void OsgViewerWidget::setViewBack() { /* similar to setViewFront, rotate 180 degrees */ }
void OsgViewerWidget::setViewLeft() { /* rotate Y-axis -90 degrees */ }
void OsgViewerWidget::setViewRight() { /* rotate Y-axis +90 degrees */ }
void OsgViewerWidget::setViewTop() { /* rotate X-axis -90 degrees */ }
void OsgViewerWidget::setViewBottom() { /* rotate X-axis +90 degrees */ }
