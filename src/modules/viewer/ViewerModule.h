#pragma once

#include "core/PluginInterface.h"
#include <QObject>

class OsgViewerWidget;

/**
 * @brief 3D 视图模块 - 管理 OSG 渲染器
 */
class ViewerModule : public QObject, public IModule
{
    Q_OBJECT
    Q_INTERFACES(IModule)

public:
    explicit ViewerModule(QObject* parent = nullptr);

    QString moduleId() const override { return "viewer"; }
    QString moduleName() const override { return "3D 视图"; }
    bool initialize() override;
    void shutdown() override;
    QList<QAction*> toolBarActions() override;

    OsgViewerWidget* viewerWidget() const { return m_viewer; }

private:
    OsgViewerWidget* m_viewer = nullptr;
};
