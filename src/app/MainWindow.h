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
#include <QTranslator>
#include <QActionGroup>

class IModule;

struct RobotLibEntry {
    QString id;
    QString name;
    QString manufacturer;
    int     dof;
    double  reach;
    double  payload;
    double  joints[6];
};

struct WorkpieceLibEntry {
    QString name;
    QString description;
    QString fileName;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void registerModule(IModule* module);

protected:
    void changeEvent(QEvent* event) override;

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

    void applyTheme(const QString& theme);
    void applyFontSize(int size);
    void switchLanguage(const QString& lang);
    void retranslateUi();

    // -- Menus --
    QMenu* m_fileMenu        = nullptr;
    QMenu* m_editMenu        = nullptr;
    QMenu* m_viewMenu        = nullptr;
    QMenu* m_toolMenu        = nullptr;
    QMenu* m_robotMenu       = nullptr;
    QMenu* m_pathMenu        = nullptr;
    QMenu* m_windowMenu      = nullptr;
    QMenu* m_helpMenu        = nullptr;
    QMenu* m_libraryMenu     = nullptr;
    QMenu* m_themeMenu       = nullptr;
    QMenu* m_fontSizeMenu    = nullptr;
    QMenu* m_langMenu        = nullptr;

    // -- Toolbars --
    QToolBar* m_fileToolBar      = nullptr;
    QToolBar* m_selectionToolBar = nullptr;
    QToolBar* m_robotToolBar     = nullptr;
    QToolBar* m_grindingToolBar  = nullptr;

    // -- Dock panels --
    QDockWidget* m_modelBrowserDock = nullptr;
    QDockWidget* m_propertyDock     = nullptr;
    QDockWidget* m_bottomDock       = nullptr;

    // -- Status bar widgets --
    QLabel* m_coordLabel     = nullptr;
    QLabel* m_rosStatusLabel = nullptr;
    QLabel* m_modelInfoLabel = nullptr;

    // -- Bottom tab widget --
    QTabWidget* m_bottomTabWidget = nullptr;

    // -- Settings state --
    QString m_currentTheme   = "dark";
    int     m_fontSize       = 12;
    QString m_currentLang    = "en";

    // -- Translator --
    QTranslator* m_translator = nullptr;

    // -- Registered modules --
    QMap<QString, IModule*> m_modules;
};
