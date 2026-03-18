#pragma once

#include <QObject>
#include <QMap>
#include <QList>
#include <memory>

class IModule;
class MainWindow;

/**
 * @brief 插件/模块管理器
 *
 * 负责模块的注册、初始化、生命周期管理。
 * 支持静态编译模块和动态加载插件两种方式。
 */
class PluginManager : public QObject
{
    Q_OBJECT

public:
    static PluginManager* instance();

    /// 注册内置模块
    void registerModule(IModule* module);

    /// 从目录加载动态插件
    void loadPlugins(const QString& pluginDir);

    /// 初始化所有已注册模块
    bool initializeAll();

    /// 关闭所有模块
    void shutdownAll();

    /// 获取指定模块
    IModule* module(const QString& moduleId) const;

    /// 获取所有模块
    QList<IModule*> allModules() const;

    /// 设置主窗口（模块会将 UI 元素注册到主窗口）
    void setMainWindow(MainWindow* window);

private:
    PluginManager(QObject* parent = nullptr);
    static PluginManager* s_instance;

    MainWindow* m_mainWindow = nullptr;
    QMap<QString, IModule*> m_modules;
    QList<QString> m_initOrder;
};
