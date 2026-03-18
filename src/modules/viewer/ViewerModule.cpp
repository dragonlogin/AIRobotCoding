#include "ViewerModule.h"
#include "OsgViewerWidget.h"
#include "core/EventBus.h"

ViewerModule::ViewerModule(QObject* parent)
    : QObject(parent)
{
}

bool ViewerModule::initialize()
{
    m_viewer = new OsgViewerWidget();

    // 转发光标移动事件到 EventBus
    connect(m_viewer, &OsgViewerWidget::cursorMoved, this,
        [](double x, double y, double z) {
            EventBus::instance()->publish("viewer.cursor.moved", {
                {"x", x}, {"y", y}, {"z", z}
            });
        });

    // 转发拾取事件
    connect(m_viewer, &OsgViewerWidget::objectPicked, this,
        [](int faceIndex) {
            EventBus::instance()->publish("cad.face.selected", {
                {"faceIndex", faceIndex}
            });
        });

    return true;
}

void ViewerModule::shutdown()
{
    delete m_viewer;
    m_viewer = nullptr;
}

QList<QAction*> ViewerModule::toolBarActions()
{
    return {};
}
