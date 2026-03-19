#include "ViewerModule.h"
#include "OsgViewerWidget.h"
#include "OccToOsgConverter.h"
#include "FacePickHandler.h"
#include "core/EventBus.h"
#include "core/DataModel.h"

#include <osg/Group>
#include <TopoDS_Shape.hxx>

ViewerModule::ViewerModule(QObject* parent)
    : QObject(parent)
{
}

QWidget* ViewerModule::viewerWidget() const { return m_viewer; }

bool ViewerModule::initialize()
{
    m_viewer = new OsgViewerWidget();
    m_converter = new OccToOsgConverter();

    // Subscribe to events
    connect(EventBus::instance(), &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            if (event == "cad.model.loaded") {
                onModelLoaded(data);
            } else if (event == "pathplan.path.generated") {
                onPathGenerated(data);
            }
        });

    return true;
}

void ViewerModule::shutdown()
{
    delete m_converter;
    m_converter = nullptr;
    delete m_viewer;
    m_viewer = nullptr;
}

QList<QAction*> ViewerModule::toolBarActions()
{
    return {};
}

void ViewerModule::onModelLoaded(const QVariantMap& data)
{
    quintptr ptr = data.value("shape_ptr").value<quintptr>();
    if (!ptr) {
        EventBus::instance()->publish("log.message", {
            {"level", "ERROR"}, {"message", "ViewerModule: no shape pointer received, cannot display model"}
        });
        return;
    }

    const TopoDS_Shape* shape = reinterpret_cast<const TopoDS_Shape*>(ptr);
    if (shape->IsNull()) {
        EventBus::instance()->publish("log.message", {
            {"level", "ERROR"}, {"message", "ViewerModule: shape is null"}
        });
        return;
    }

    // OCC shape -> OSG mesh
    osg::ref_ptr<osg::Group> node = m_converter->convertShape(*shape);

    // Replace the old model in the scene
    m_viewer->removeSceneNode("CADModel");
    m_viewer->addSceneNode(node.get(), "CADModel");

    // Fit view to model
    m_viewer->fitAll();

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"}, {"message", "3D model displayed successfully"}
    });
}

void ViewerModule::onPathGenerated(const QVariantMap& data)
{
    Q_UNUSED(data)

    // Fetch path data from DataModel and render as OSG line strips
    const auto& tasks = DataModel::instance()->tasks();
    if (tasks.isEmpty()) return;

    // Path visualization is rendered in OsgViewerWidget via pathGroup
    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "Path visualization updated"}
    });
}
