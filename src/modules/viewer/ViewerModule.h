#pragma once

#include "core/PluginInterface.h"
#include <QObject>
#include <QWidget>

class OsgViewerWidget;
class OccToOsgConverter;

/**
 * @brief 3D Viewer Module - manages the OSG renderer
 */
class ViewerModule : public QObject, public IModule
{
    Q_OBJECT
    Q_INTERFACES(IModule)

public:
    explicit ViewerModule(QObject* parent = nullptr);

    QString moduleId() const override { return "viewer"; }
    QString moduleName() const override { return "3D Viewer"; }
    bool initialize() override;
    void shutdown() override;
    QList<QAction*> toolBarActions() override;

    QWidget* viewerWidget() const;

private:
    void onModelLoaded(const QVariantMap& data);
    void onPathGenerated(const QVariantMap& data);

    OsgViewerWidget* m_viewer = nullptr;
    OccToOsgConverter* m_converter = nullptr;
};
