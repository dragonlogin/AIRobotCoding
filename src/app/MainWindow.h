#pragma once

#include <QMainWindow>
#include <QDockWidget>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QTabWidget>
#include <QLabel>
#include <QMap>
#include <QString>

class IModule;

struct RobotLibEntry {
    QString id;
    QString name;
    QString manufacturer;
    int     dof;
    double  reach;      // mm
    double  payload;    // kg
    double  joints[6];  // Test initial joint angles (deg)
};

struct WorkpieceLibEntry {
    QString name;
    QString description;
    QString fileName;   // File name relative to resources/workpieces/
};

/**
 * @brief Main window - responsible for layout management and module integration
 *
 * Uses Qt DockWidget for a flexible panel layout;
 * all functional panels can be dragged, floated, and hidden.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    /// Register a module and integrate its UI elements
    void registerModule(IModule* module);

private:
    void setupMenuBar();
    void setupToolBars();
    void setupStatusBar();
    void setupCentralWidget();
    void setupDockWidgets();
    void setupConnections();
    void loadStyleSheet();
    void setupLibraryMenus(QMenu* libraryMenu);
    void loadRobotFromLibrary(const RobotLibEntry& entry);
    void loadWorkpieceFromLibrary(const WorkpieceLibEntry& entry);

    // -- Menus --
    QMenu* m_fileMenu = nullptr;
    QMenu* m_editMenu = nullptr;
    QMenu* m_viewMenu = nullptr;
    QMenu* m_toolMenu = nullptr;
    QMenu* m_robotMenu = nullptr;
    QMenu* m_pathMenu = nullptr;
    QMenu* m_windowMenu = nullptr;
    QMenu* m_helpMenu = nullptr;
    QMenu* m_libraryMenu = nullptr;

    // -- Toolbars --
    QToolBar* m_fileToolBar = nullptr;
    QToolBar* m_selectionToolBar = nullptr;
    QToolBar* m_robotToolBar = nullptr;
    QToolBar* m_grindingToolBar = nullptr;

    // -- Dock panels --
    QDockWidget* m_modelBrowserDock = nullptr;
    QDockWidget* m_propertyDock = nullptr;
    QDockWidget* m_bottomDock = nullptr;

    // -- Status bar widgets --
    QLabel* m_coordLabel = nullptr;
    QLabel* m_rosStatusLabel = nullptr;
    QLabel* m_modelInfoLabel = nullptr;

    // -- Bottom tab widget --
    QTabWidget* m_bottomTabWidget = nullptr;

    // -- Registered modules --
    QMap<QString, IModule*> m_modules;
};
