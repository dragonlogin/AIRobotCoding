#include "PluginManager.h"
#include "MainWindow.h"
#include "core/PluginInterface.h"

#include <QDir>
#include <QPluginLoader>
#include <QDebug>

PluginManager* PluginManager::s_instance = nullptr;

PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
{
}

PluginManager* PluginManager::instance()
{
    if (!s_instance) {
        s_instance = new PluginManager();
    }
    return s_instance;
}

void PluginManager::registerModule(IModule* module)
{
    if (!module)
        return;

    const QString id = module->moduleId();
    if (m_modules.contains(id)) {
        qWarning() << "Module already registered:" << id;
        return;
    }

    m_modules.insert(id, module);
    m_initOrder.append(id);
    qInfo() << "Module registered:" << id << "-" << module->moduleName();
}

void PluginManager::loadPlugins(const QString& pluginDir)
{
    QDir dir(pluginDir);
    for (const QString& fileName : dir.entryList(QDir::Files)) {
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        QObject* plugin = loader.instance();
        if (plugin) {
            IModule* module = qobject_cast<IModule*>(plugin);
            if (module) {
                registerModule(module);
            }
        }
    }
}

bool PluginManager::initializeAll()
{
    for (const QString& id : m_initOrder) {
        IModule* mod = m_modules.value(id);
        if (!mod->initialize()) {
            qCritical() << "Failed to initialize module:" << id;
            return false;
        }

        if (m_mainWindow) {
            m_mainWindow->registerModule(mod);
        }

        qInfo() << "Module initialized:" << id;
    }
    return true;
}

void PluginManager::shutdownAll()
{
    // 逆序关闭
    for (int i = m_initOrder.size() - 1; i >= 0; --i) {
        IModule* mod = m_modules.value(m_initOrder[i]);
        mod->shutdown();
        qInfo() << "Module shutdown:" << m_initOrder[i];
    }
}

IModule* PluginManager::module(const QString& moduleId) const
{
    return m_modules.value(moduleId, nullptr);
}

QList<IModule*> PluginManager::allModules() const
{
    return m_modules.values();
}

void PluginManager::setMainWindow(MainWindow* window)
{
    m_mainWindow = window;
}
