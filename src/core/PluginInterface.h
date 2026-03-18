#pragma once

#include <QString>
#include <QWidget>
#include <QtPlugin>
#include <QList>
#include <QAction>

/**
 * @brief 模块/插件接口基类
 *
 * 所有功能模块（CAD、Viewer、Robot、PathPlan、Grinding）都实现此接口，
 * 通过 PluginManager 统一加载和管理，实现低耦合高扩展。
 */
class IModule
{
public:
    virtual ~IModule() = default;

    /// 模块唯一标识
    virtual QString moduleId() const = 0;

    /// 模块显示名称
    virtual QString moduleName() const = 0;

    /// 模块初始化（依赖注入、信号连接等）
    virtual bool initialize() = 0;

    /// 模块卸载清理
    virtual void shutdown() = 0;

    /// 返回该模块提供的停靠面板列表（可为空）
    virtual QList<QWidget*> dockWidgets() { return {}; }

    /// 返回该模块提供的工具栏 Action 列表（可为空）
    virtual QList<QAction*> toolBarActions() { return {}; }

    /// 返回该模块的菜单 Action 列表（可为空）
    virtual QList<QAction*> menuActions() { return {}; }
};

#define IModule_iid "com.airobot.grinding.IModule/1.0"
Q_DECLARE_INTERFACE(IModule, IModule_iid)
