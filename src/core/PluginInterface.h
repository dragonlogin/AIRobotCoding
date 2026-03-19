#pragma once

#include <QString>
#include <QWidget>
#include <QtPlugin>
#include <QList>
#include <QAction>

/**
 * @brief Base interface for modules/plugins
 *
 * All functional modules (CAD, Viewer, Robot, PathPlan, Grinding) implement this interface,
 * loaded and managed uniformly by PluginManager for low coupling and high extensibility.
 */
class IModule
{
public:
    virtual ~IModule() = default;

    /// Unique module identifier
    virtual QString moduleId() const = 0;

    /// Module display name
    virtual QString moduleName() const = 0;

    /// Module initialization (dependency injection, signal connections, etc.)
    virtual bool initialize() = 0;

    /// Module shutdown and cleanup
    virtual void shutdown() = 0;

    /// Returns the list of dock panels provided by this module (may be empty)
    virtual QList<QWidget*> dockWidgets() { return {}; }

    /// Returns the list of toolbar actions provided by this module (may be empty)
    virtual QList<QAction*> toolBarActions() { return {}; }

    /// Returns the list of menu actions provided by this module (may be empty)
    virtual QList<QAction*> menuActions() { return {}; }
};

#define IModule_iid "com.airobot.grinding.IModule/1.0"
Q_DECLARE_INTERFACE(IModule, IModule_iid)
