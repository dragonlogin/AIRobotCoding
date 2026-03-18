#pragma once

#include <QMainWindow>
#include <QDockWidget>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QTabWidget>
#include <QLabel>
#include <QMap>

class IModule;

/**
 * @brief 主窗口 - 负责布局管理和模块集成
 *
 * 采用 Qt DockWidget 实现灵活的面板布局，
 * 所有功能面板均可拖拽、浮动、隐藏。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    /// 注册模块并集成其 UI 元素
    void registerModule(IModule* module);

private:
    void setupMenuBar();
    void setupToolBars();
    void setupStatusBar();
    void setupCentralWidget();
    void setupDockWidgets();
    void setupConnections();
    void loadStyleSheet();

    // -- 菜单 --
    QMenu* m_fileMenu = nullptr;
    QMenu* m_editMenu = nullptr;
    QMenu* m_viewMenu = nullptr;
    QMenu* m_toolMenu = nullptr;
    QMenu* m_robotMenu = nullptr;
    QMenu* m_pathMenu = nullptr;
    QMenu* m_windowMenu = nullptr;
    QMenu* m_helpMenu = nullptr;

    // -- 工具栏 --
    QToolBar* m_fileToolBar = nullptr;
    QToolBar* m_selectionToolBar = nullptr;
    QToolBar* m_robotToolBar = nullptr;
    QToolBar* m_grindingToolBar = nullptr;

    // -- 停靠面板 --
    QDockWidget* m_modelBrowserDock = nullptr;
    QDockWidget* m_propertyDock = nullptr;
    QDockWidget* m_bottomDock = nullptr;

    // -- 状态栏组件 --
    QLabel* m_coordLabel = nullptr;
    QLabel* m_rosStatusLabel = nullptr;
    QLabel* m_modelInfoLabel = nullptr;

    // -- 底部标签页 --
    QTabWidget* m_bottomTabWidget = nullptr;

    // -- 已注册模块 --
    QMap<QString, IModule*> m_modules;
};
