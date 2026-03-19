#pragma once

#include <QObject>
#include <QMap>
#include <QList>
#include <memory>

class IModule;
class MainWindow;

/**
 * @brief Plugin/module manager
 *
 * Responsible for module registration, initialization, and lifecycle management.
 * Supports both statically compiled modules and dynamically loaded plugins.
 */
class PluginManager : public QObject
{
    Q_OBJECT

public:
    static PluginManager* instance();

    /// Register a built-in module
    void registerModule(IModule* module);

    /// Load dynamic plugins from a directory
    void loadPlugins(const QString& pluginDir);

    /// Initialize all registered modules
    bool initializeAll();

    /// Shut down all modules
    void shutdownAll();

    /// Get a specific module by ID
    IModule* module(const QString& moduleId) const;

    /// Get all modules
    QList<IModule*> allModules() const;

    /// Set the main window (modules register their UI elements into it)
    void setMainWindow(MainWindow* window);

private:
    PluginManager(QObject* parent = nullptr);
    static PluginManager* s_instance;

    MainWindow* m_mainWindow = nullptr;
    QMap<QString, IModule*> m_modules;
    QList<QString> m_initOrder;
};
