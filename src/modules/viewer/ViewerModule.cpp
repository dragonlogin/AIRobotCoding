#include "ViewerModule.h"
#include "OsgViewerWidget.h"
#include "OccToOsgConverter.h"
#include "FacePickHandler.h"
#include "core/EventBus.h"
#include "core/DataModel.h"

#include <osg/Group>

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
    Q_UNUSED(data)

    // 从 EventBus 传递的数据中无法直接获取 TopoDS_Shape
    // 需要从 CadModule 获取，这里通过 PluginManager 查找
    // 为保持解耦，使用事件总线请求数据
    EventBus::instance()->publish("cad.shape.request", {});
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
