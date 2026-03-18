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

    // 监听事件
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
            {"level", "ERROR"}, {"message", "ViewerModule: 未收到 shape 指针，无法显示模型"}
        });
        return;
    }

    const TopoDS_Shape* shape = reinterpret_cast<const TopoDS_Shape*>(ptr);
    if (shape->IsNull()) {
        EventBus::instance()->publish("log.message", {
            {"level", "ERROR"}, {"message", "ViewerModule: shape 为空"}
        });
        return;
    }

    // OCC shape → OSG 网格
    osg::ref_ptr<osg::Group> node = m_converter->convertShape(*shape);

    // 替换场景中的旧模型
    m_viewer->removeSceneNode("CADModel");
    m_viewer->addSceneNode(node.get(), "CADModel");

    // 视图适配
    m_viewer->fitAll();

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"}, {"message", "3D 模型显示完成"}
    });
}

void ViewerModule::onPathGenerated(const QVariantMap& data)
{
    Q_UNUSED(data)

    // 从 DataModel 获取路径数据，转换为 OSG 线条显示
    const auto& tasks = DataModel::instance()->tasks();
    if (tasks.isEmpty()) return;

    // 路径可视化将在 OsgViewerWidget 中通过 pathGroup 渲染
    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "路径可视化已更新"}
    });
}
