#include "MainWindow.h"
#include "core/PluginInterface.h"
#include "core/EventBus.h"
#include "core/DataModel.h"
#include "ui/panels/ModelBrowserPanel.h"
#include "ui/panels/PropertyPanel.h"
#include "ui/panels/LogPanel.h"
#include "ui/panels/RosMonitorPanel.h"
#include "ui/panels/PathDataPanel.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QDir>
#include <QSettings>
#include <QEvent>
#include <QRegularExpression>
#ifdef _WIN32
#  define _USE_MATH_DEFINES
#endif
#include <cmath>

static QString findResourceDir(const QString& subDir);
static QString robotsResourceDir();
static QString workpiecesResourceDir();

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_translator(new QTranslator(this))
{
    // Restore settings
    QSettings s("AIRobot", "AIRobotGrinding");
    m_currentTheme = s.value("theme", "dark").toString();
    m_fontSize      = s.value("fontSize", 12).toInt();
    m_currentLang   = s.value("language", "en").toString();

    resize(1600, 900);
    setMinimumSize(1200, 700);
    menuBar()->setNativeMenuBar(false);

    setupMenuBar();
    setupToolBars();
    setupCentralWidget();
    setupDockWidgets();
    setupStatusBar();
    setupConnections();
    loadStyleSheet();
    retranslateUi();

    resizeDocks({m_modelBrowserDock}, {250}, Qt::Horizontal);
    resizeDocks({m_propertyDock},     {300}, Qt::Horizontal);
    resizeDocks({m_bottomDock},       {180}, Qt::Vertical);
}

MainWindow::~MainWindow() {}

void MainWindow::registerModule(IModule* module)
{
    if (!module || m_modules.contains(module->moduleId()))
        return;
    m_modules.insert(module->moduleId(), module);
    for (QAction* action : module->toolBarActions())
        m_grindingToolBar->addAction(action);
    for (QAction* action : module->menuActions())
        m_toolMenu->addAction(action);
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QMainWindow::changeEvent(event);
}

// ============================================================================
// Menu bar
// ============================================================================
void MainWindow::setupMenuBar()
{
    QMenuBar* mb = menuBar();

    m_fileMenu   = mb->addMenu("placeholder");
    m_editMenu   = mb->addMenu("placeholder");
    m_viewMenu   = mb->addMenu("placeholder");
    m_toolMenu   = mb->addMenu("placeholder");
    m_robotMenu  = mb->addMenu("placeholder");
    m_pathMenu   = mb->addMenu("placeholder");
    m_windowMenu = mb->addMenu("placeholder");
    m_libraryMenu= mb->addMenu("placeholder");
    m_helpMenu   = mb->addMenu("placeholder");

    // -- File --
    QAction* importAction = m_fileMenu->addAction("placeholder");
    importAction->setShortcut(QKeySequence("Ctrl+I"));
    m_fileMenu->addAction("placeholder"); // Import Robot
    m_fileMenu->addSeparator();
    m_fileMenu->addAction("placeholder")->setShortcut(QKeySequence::Save); // Save
    m_fileMenu->addAction("placeholder"); // Save As
    m_fileMenu->addSeparator();
    m_fileMenu->addAction("placeholder"); // Export path
    m_fileMenu->addAction("placeholder"); // Export report
    m_fileMenu->addSeparator();
    QAction* exitAction = m_fileMenu->addAction("placeholder");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    connect(importAction, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this, tr("Import STEP Model"), "",
            tr("STEP Files (*.step *.stp);;All Files (*.*)"));
        if (!path.isEmpty())
            EventBus::instance()->publish("cad.import.request", {{"path", path}});
    });

    // -- Edit --
    m_editMenu->addAction("placeholder")->setShortcut(QKeySequence::Undo);
    m_editMenu->addAction("placeholder")->setShortcut(QKeySequence::Redo);
    m_editMenu->addSeparator();
    m_editMenu->addAction("placeholder");

    // -- View --
    m_viewMenu->addAction("placeholder")->setShortcut(QKeySequence("1"));
    m_viewMenu->addAction("placeholder")->setShortcut(QKeySequence("2"));
    m_viewMenu->addAction("placeholder")->setShortcut(QKeySequence("3"));
    m_viewMenu->addAction("placeholder")->setShortcut(QKeySequence("4"));
    m_viewMenu->addAction("placeholder")->setShortcut(QKeySequence("5"));
    m_viewMenu->addAction("placeholder")->setShortcut(QKeySequence("6"));
    m_viewMenu->addAction("placeholder")->setShortcut(QKeySequence("0"));
    m_viewMenu->addSeparator();
    m_viewMenu->addAction("placeholder")->setShortcut(QKeySequence("F"));
    m_viewMenu->addSeparator();
    QAction* wireframe = m_viewMenu->addAction("placeholder");
    wireframe->setCheckable(true);
    QAction* shaded = m_viewMenu->addAction("placeholder");
    shaded->setCheckable(true);
    shaded->setChecked(true);
    QAction* transparent = m_viewMenu->addAction("placeholder");
    transparent->setCheckable(true);
    m_viewMenu->addSeparator();

    // Theme submenu
    m_themeMenu = m_viewMenu->addMenu("placeholder");
    QActionGroup* themeGroup = new QActionGroup(this);
    QAction* darkAct  = m_themeMenu->addAction("placeholder");
    QAction* lightAct = m_themeMenu->addAction("placeholder");
    darkAct->setCheckable(true);
    lightAct->setCheckable(true);
    darkAct->setChecked(m_currentTheme == "dark");
    lightAct->setChecked(m_currentTheme == "light");
    themeGroup->addAction(darkAct);
    themeGroup->addAction(lightAct);
    connect(darkAct,  &QAction::triggered, this, [this]{ applyTheme("dark");  });
    connect(lightAct, &QAction::triggered, this, [this]{ applyTheme("light"); });

    // Font size submenu
    m_fontSizeMenu = m_viewMenu->addMenu("placeholder");
    QActionGroup* fontGroup = new QActionGroup(this);
    for (int sz : {9, 10, 11, 12, 13, 14, 16}) {
        QAction* a = m_fontSizeMenu->addAction(QString("%1 px").arg(sz));
        a->setCheckable(true);
        a->setChecked(sz == m_fontSize);
        a->setData(sz);
        fontGroup->addAction(a);
        connect(a, &QAction::triggered, this, [this, sz]{ applyFontSize(sz); });
    }

    // Language submenu
    m_langMenu = m_viewMenu->addMenu("placeholder");
    QActionGroup* langGroup = new QActionGroup(this);
    QAction* enAct = m_langMenu->addAction("English");
    QAction* zhAct = m_langMenu->addAction("中文");
    enAct->setCheckable(true);
    zhAct->setCheckable(true);
    enAct->setChecked(m_currentLang == "en");
    zhAct->setChecked(m_currentLang == "zh");
    langGroup->addAction(enAct);
    langGroup->addAction(zhAct);
    connect(enAct, &QAction::triggered, this, [this]{ switchLanguage("en"); });
    connect(zhAct, &QAction::triggered, this, [this]{ switchLanguage("zh"); });

    // -- Tools --
    m_toolMenu->addAction("placeholder");
    m_toolMenu->addAction("placeholder");
    m_toolMenu->addAction("placeholder");
    m_toolMenu->addAction("placeholder");

    // -- Robot --
    m_robotMenu->addAction("placeholder");
    m_robotMenu->addAction("placeholder");
    m_robotMenu->addSeparator();
    m_robotMenu->addAction("placeholder");
    m_robotMenu->addAction("placeholder");
    m_robotMenu->addAction("placeholder");
    m_robotMenu->addSeparator();
    QAction* eStop = m_robotMenu->addAction("placeholder");
    eStop->setShortcut(QKeySequence("Escape"));

    // -- Path --
    m_pathMenu->addAction("placeholder");
    m_pathMenu->addAction("placeholder");
    m_pathMenu->addAction("placeholder");
    m_pathMenu->addSeparator();
    m_pathMenu->addAction("placeholder")->setShortcut(QKeySequence("F5"));
    m_pathMenu->addAction("placeholder")->setShortcut(QKeySequence("Shift+F5"));
    m_pathMenu->addSeparator();
    m_pathMenu->addAction("placeholder")->setShortcut(QKeySequence("F6"));

    // -- Library --
    setupLibraryMenus(m_libraryMenu);

    // -- Help --
    m_helpMenu->addAction("placeholder");
    m_helpMenu->addAction("placeholder");
}

// ============================================================================
// retranslateUi — updates all translatable strings after language change
// ============================================================================
void MainWindow::retranslateUi()
{
    setWindowTitle(tr("AIRobot Surface Grinding System v1.0"));

    // Menus
    m_fileMenu->setTitle(tr("File(&F)"));
    m_editMenu->setTitle(tr("Edit(&E)"));
    m_viewMenu->setTitle(tr("View(&V)"));
    m_toolMenu->setTitle(tr("Tools(&T)"));
    m_robotMenu->setTitle(tr("Robot(&R)"));
    m_pathMenu->setTitle(tr("Path(&P)"));
    m_windowMenu->setTitle(tr("Window(&W)"));
    m_libraryMenu->setTitle(tr("Library(&L)"));
    m_helpMenu->setTitle(tr("Help(&H)"));

    // File menu items (by index)
    auto fa = m_fileMenu->actions();
    if (fa.size() >= 9) {
        fa[0]->setText(tr("Import STEP Model(&I)..."));
        fa[1]->setText(tr("Import Robot Model(&R)..."));
        fa[3]->setText(tr("Save Project(&S)"));
        fa[4]->setText(tr("Save As(&A)..."));
        fa[6]->setText(tr("Export Grinding Path..."));
        fa[7]->setText(tr("Export Report..."));
        fa[9]->setText(tr("Exit(&X)"));
    }

    // Edit menu items
    auto ea = m_editMenu->actions();
    if (ea.size() >= 3) {
        ea[0]->setText(tr("Undo(&U)"));
        ea[1]->setText(tr("Redo(&R)"));
        ea[3]->setText(tr("Preferences(&P)..."));
    }

    // View menu items
    auto va = m_viewMenu->actions();
    if (va.size() >= 16) {
        va[0]->setText(tr("Front View"));
        va[1]->setText(tr("Back View"));
        va[2]->setText(tr("Left View"));
        va[3]->setText(tr("Right View"));
        va[4]->setText(tr("Top View"));
        va[5]->setText(tr("Bottom View"));
        va[6]->setText(tr("Isometric View"));
        va[8]->setText(tr("Fit to Window(&F)"));
        va[10]->setText(tr("Wireframe Mode"));
        va[11]->setText(tr("Shaded Mode"));
        va[12]->setText(tr("Transparent Mode"));
    }
    if (m_themeMenu)  m_themeMenu->setTitle(tr("Theme"));
    if (m_fontSizeMenu) m_fontSizeMenu->setTitle(tr("Font Size"));
    if (m_langMenu)   m_langMenu->setTitle(tr("Language"));

    if (m_themeMenu && m_themeMenu->actions().size() >= 2) {
        m_themeMenu->actions()[0]->setText(tr("Dark Theme"));
        m_themeMenu->actions()[1]->setText(tr("Light Theme"));
    }

    // Tools menu items
    auto ta = m_toolMenu->actions();
    if (ta.size() >= 4) {
        ta[0]->setText(tr("Measure Distance"));
        ta[1]->setText(tr("Measure Angle"));
        ta[2]->setText(tr("Section Analysis"));
        ta[3]->setText(tr("Curvature Analysis"));
    }

    // Robot menu items
    auto ra = m_robotMenu->actions();
    if (ra.size() >= 7) {
        ra[0]->setText(tr("Connect to ROS Master..."));
        ra[1]->setText(tr("Disconnect"));
        ra[3]->setText(tr("Load Robot URDF..."));
        ra[4]->setText(tr("Teaching Mode"));
        ra[5]->setText(tr("Manual Control"));
        ra[7]->setText(tr("Emergency Stop(&E)"));
    }

    // Path menu items
    auto pa = m_pathMenu->actions();
    if (pa.size() >= 7) {
        pa[0]->setText(tr("Generate Grinding Path..."));
        pa[1]->setText(tr("Path Optimization..."));
        pa[2]->setText(tr("Collision Detection"));
        pa[4]->setText(tr("Run Simulation(&S)"));
        pa[5]->setText(tr("Stop Simulation"));
        pa[7]->setText(tr("Execute Grinding(&X)"));
    }

    // Window menu
    auto wa = m_windowMenu->actions();
    for (QAction* a : wa) {
        if (a->text().contains("Reset") || a->text().contains("重置"))
            a->setText(tr("Reset Layout"));
    }

    // Library submenus
    auto la = m_libraryMenu->actions();
    for (QAction* a : la) {
        if (a->menu()) {
            QString t = a->menu()->title();
            if (t == "Robot Library" || t == tr("Robot Library"))
                a->setText(tr("Robot Library"));
            else if (t == "STEP Workpiece Library" || t == tr("STEP Workpiece Library"))
                a->setText(tr("STEP Workpiece Library"));
        }
    }

    // Help menu items
    auto ha = m_helpMenu->actions();
    if (ha.size() >= 2) {
        ha[0]->setText(tr("User Manual"));
        ha[1]->setText(tr("About(&A)..."));
    }

    // Toolbars
    if (m_fileToolBar) {
        auto tfa = m_fileToolBar->actions();
        if (tfa.size() >= 4) {
            tfa[0]->setText(tr("Import"));
            tfa[1]->setText(tr("Save"));
            tfa[3]->setText(tr("Undo"));
            tfa[4]->setText(tr("Redo"));
        }
    }
    if (m_selectionToolBar) {
        auto sa = m_selectionToolBar->actions();
        if (sa.size() >= 4) {
            sa[0]->setText(tr("Select Face"));
            sa[1]->setText(tr("Select Edge"));
            sa[2]->setText(tr("Select Solid"));
            sa[3]->setText(tr("Box Select"));
        }
    }
    if (m_robotToolBar) {
        auto rta = m_robotToolBar->actions();
        if (rta.size() >= 2) {
            rta[0]->setText(tr("Connect"));
            rta[1]->setText(tr("Emergency Stop"));
            rta[1]->setToolTip(tr("Emergency Stop (Esc)"));
        }
    }
    if (m_grindingToolBar) {
        auto gta = m_grindingToolBar->actions();
        if (gta.size() >= 5) {
            gta[0]->setText(tr("Generate Path"));
            gta[1]->setText(tr("Simulate"));
            gta[2]->setText(tr("Execute"));
            gta[4]->setText(tr("Measure"));
            gta[5]->setText(tr("Section"));
        }
    }

    // Toolbar titles
    if (m_fileToolBar)      m_fileToolBar->setWindowTitle(tr("File"));
    if (m_selectionToolBar) m_selectionToolBar->setWindowTitle(tr("Selection"));
    if (m_robotToolBar)     m_robotToolBar->setWindowTitle(tr("Robot"));
    if (m_grindingToolBar)  m_grindingToolBar->setWindowTitle(tr("Grinding"));

    // Dock widgets
    if (m_modelBrowserDock) m_modelBrowserDock->setWindowTitle(tr("Model Browser"));
    if (m_propertyDock)     m_propertyDock->setWindowTitle(tr("Properties"));
    if (m_bottomDock)       m_bottomDock->setWindowTitle(tr("Output"));

    // Bottom tabs
    if (m_bottomTabWidget && m_bottomTabWidget->count() >= 3) {
        m_bottomTabWidget->setTabText(0, tr("Log"));
        m_bottomTabWidget->setTabText(1, tr("ROS Topics"));
        m_bottomTabWidget->setTabText(2, tr("Path Data"));
    }

    // Status bar
    if (m_coordLabel)
        m_coordLabel->setText(tr("Coordinate: X:%1 Y:%2 Z:%3").arg(0.0,0,'f',1).arg(0.0,0,'f',1).arg(0.0,0,'f',1));
    if (m_rosStatusLabel)
        m_rosStatusLabel->setText(tr("ROS: Not Connected"));
    if (m_modelInfoLabel)
        m_modelInfoLabel->setText(tr("Model: None"));
}

// ============================================================================
// Theme / font size / language
// ============================================================================
void MainWindow::applyTheme(const QString& theme)
{
    m_currentTheme = theme;
    QSettings("AIRobot", "AIRobotGrinding").setValue("theme", theme);
    loadStyleSheet();

    // Notify viewer to update background color
    if (theme == "dark") {
        EventBus::instance()->publish("viewer.background.changed",
            {{"r", 0.17}, {"g", 0.17}, {"b", 0.17}});
    } else {
        EventBus::instance()->publish("viewer.background.changed",
            {{"r", 0.85}, {"g", 0.85}, {"b", 0.85}});
    }
}

void MainWindow::applyFontSize(int size)
{
    m_fontSize = size;
    QSettings("AIRobot", "AIRobotGrinding").setValue("fontSize", size);
    loadStyleSheet();
}

void MainWindow::switchLanguage(const QString& lang)
{
    m_currentLang = lang;
    QSettings("AIRobot", "AIRobotGrinding").setValue("language", lang);

    qApp->removeTranslator(m_translator);
    if (lang == "zh") {
        if (m_translator->load("zh_CN", qApp->applicationDirPath() + "/translations"))
            qApp->installTranslator(m_translator);
    }
    // changeEvent(LanguageChange) is fired automatically by installTranslator /
    // removeTranslator, which calls retranslateUi() via our changeEvent override.
}

// ============================================================================
// Style sheet  (theme + dynamic font size)
// ============================================================================
void MainWindow::loadStyleSheet()
{
    QString path = (m_currentTheme == "light")
                   ? ":/styles/light_theme.qss"
                   : ":/styles/dark_theme.qss";
    QFile f(path);
    if (!f.open(QFile::ReadOnly | QFile::Text))
        return;
    QString qss = QString::fromUtf8(f.readAll());
    f.close();

    // Replace every occurrence of "font-size: Npx" with the user-chosen size
    qss.replace(QRegularExpression(R"(font-size:\s*\d+px)"),
                QString("font-size: %1px").arg(m_fontSize));

    setStyleSheet(qss);
}

// ============================================================================
// Robot library & STEP workpiece library
// ============================================================================
void MainWindow::setupLibraryMenus(QMenu* libraryMenu)
{
    static const RobotLibEntry robotLib[] = {
        { "ur5",      "UR5",                    "Universal Robots", 6,  850,  5,  { 0, -90,  90, -90,  0, 0 } },
        { "ur10",     "UR10",                   "Universal Robots", 6, 1300, 10,  { 0, -90,  90, -90,  0, 0 } },
        { "kr6_r900", "KR6 R900",               "KUKA",             6,  900,  6,  { 0,  45,  90,   0, 90, 0 } },
        { "irb1200",  "IRB 1200",               "ABB",              6,  700,  5,  { 0,  30,  60,   0, 60, 0 } },
        { "m10ia",    "M10iA",                  "Fanuc",            6, 1422, 10,  { 0, -60,  80,   0, 70, 0 } },
        { "cr7ia",    "CR-7iA (Collaborative)", "Fanuc",            6,  717,  7,  { 0, -45,  90,   0, 45, 0 } },
    };

    QMenu* robotLibMenu = libraryMenu->addMenu(tr("Robot Library"));
    for (const auto& entry : robotLib) {
        QString label = QString("%1  [%2]  Reach %3mm / %4kg")
                            .arg(entry.name, entry.manufacturer)
                            .arg(entry.reach).arg(entry.payload);
        QAction* act = robotLibMenu->addAction(label);
        connect(act, &QAction::triggered, this, [this, entry]() {
            loadRobotFromLibrary(entry);
        });
    }

    robotLibMenu->addSeparator();
    QAction* kukaStepAct = robotLibMenu->addAction("KUKA KR600 R2830  [3D Model]  Import STEP...");
    connect(kukaStepAct, &QAction::triggered, this, [this]() {
        QString baseDir  = robotsResourceDir();
        QString filePath = QDir(baseDir).filePath("KR600_R2830.stp");
        if (!QFile::exists(filePath))
            filePath = QFileDialog::getOpenFileName(this, tr("Load Robot STEP Model"),
                            baseDir, tr("STEP Files (*.step *.stp);;All Files (*.*)"));
        if (!filePath.isEmpty())
            EventBus::instance()->publish("cad.import.request", {{"path", filePath}});
    });

    libraryMenu->addSeparator();

    static const WorkpieceLibEntry workpieceLib[] = {
        { "Multi-Feature Mechanical Part",  "Planes/holes/slots, AP214 standard test piece",                      "flat_plate.stp"      },
        { "Cylindrical Assembly",           "Multi-part assembly with cylindrical surfaces",                      "cylinder.stp"        },
        { "B-Spline Cage Surface",          "Spline surface structure, suitable for path planning verification",  "sphere_half.stp"     },
        { "Ventilation Impeller",           "Complex rotational surface, turbine blade-like structure",           "turbine_blade.stp"   },
        { "Multi-Face Recognition Part",    "Multiple surface types, suitable for surface recognition testing",   "freeform_surface.stp"},
        { "Complex Assembly",               "Large freeform surface assembly, high face count test",              "complex_surface.stp" },
        { "Standard Assembly Test Piece",   "AP214 assembly standard test piece",                                 "assembly_test.stp"   },
    };

    QMenu* workpieceLibMenu = libraryMenu->addMenu(tr("STEP Workpiece Library"));
    for (const auto& entry : workpieceLib) {
        QString label = QString("%1  —  %2").arg(entry.name, entry.description);
        QAction* act = workpieceLibMenu->addAction(label);
        connect(act, &QAction::triggered, this, [this, entry]() {
            loadWorkpieceFromLibrary(entry);
        });
    }
}

void MainWindow::loadRobotFromLibrary(const RobotLibEntry& entry)
{
    RobotState state;
    state.connected  = true;
    state.moving     = false;
    state.statusText = QString("[Library] %1 (%2) loaded").arg(entry.name, entry.manufacturer);
    for (int i = 0; i < 6; ++i)
        state.joints[i] = entry.joints[i];
    double j1r = entry.joints[1] * M_PI / 180.0;
    state.tcpPosition = QVector3D(
        static_cast<float>(entry.reach * 0.6), 0.0f,
        static_cast<float>(entry.reach * std::abs(std::sin(j1r))));

    DataModel::instance()->updateRobotState(state);
    m_rosStatusLabel->setText(tr("Robot: %1").arg(entry.name));
    EventBus::instance()->publish("log.message", {
        {"level",   "INFO"},
        {"message", QString("Robot loaded from library: %1 | Manufacturer: %2 | DOF: %3 | Reach: %4 mm")
             .arg(entry.name, entry.manufacturer).arg(entry.dof).arg(entry.reach)}
    });
}

static QString findResourceDir(const QString& subDir)
{
    const QString exe = QCoreApplication::applicationDirPath();
    for (const QString& rel : { "../resources/" + subDir,
                                 "../../resources/" + subDir,
                                 "../../../resources/" + subDir,
                                 "resources/" + subDir }) {
        QString candidate = QDir(exe).filePath(rel);
        if (QDir(candidate).exists())
            return QDir(candidate).absolutePath();
    }
    return QDir(exe).filePath("../resources/" + subDir);
}
static QString robotsResourceDir()     { return findResourceDir("robots");     }
static QString workpiecesResourceDir() { return findResourceDir("workpieces"); }

void MainWindow::loadWorkpieceFromLibrary(const WorkpieceLibEntry& entry)
{
    QString baseDir  = workpiecesResourceDir();
    QString filePath = QDir(baseDir).filePath(entry.fileName);
    if (!QFile::exists(filePath)) {
        filePath = QFileDialog::getOpenFileName(
            this, tr("Import Workpiece: %1").arg(entry.name),
            baseDir, tr("STEP Files (*.step *.stp);;All Files (*.*)"));
    }
    if (filePath.isEmpty()) return;

    EventBus::instance()->publish("cad.import.request", {{"path", filePath}});
    EventBus::instance()->publish("log.message", {
        {"level",   "INFO"},
        {"message", QString("Workpiece loaded from library: %1 — %2").arg(entry.name, entry.description)}
    });
}

// ============================================================================
// Toolbars
// ============================================================================
void MainWindow::setupToolBars()
{
    m_fileToolBar = addToolBar("File");
    m_fileToolBar->setObjectName("FileToolBar");
    m_fileToolBar->setIconSize(QSize(24, 24));
    m_fileToolBar->addAction("Import");
    m_fileToolBar->addAction("Save");
    m_fileToolBar->addSeparator();
    m_fileToolBar->addAction("Undo");
    m_fileToolBar->addAction("Redo");

    m_selectionToolBar = addToolBar("Selection");
    m_selectionToolBar->setObjectName("SelectionToolBar");
    m_selectionToolBar->addAction("Select Face");
    m_selectionToolBar->addAction("Select Edge");
    m_selectionToolBar->addAction("Select Solid");
    m_selectionToolBar->addAction("Box Select");

    m_robotToolBar = addToolBar("Robot");
    m_robotToolBar->setObjectName("RobotToolBar");
    m_robotToolBar->addAction("Connect");
    QAction* eStopBtn = m_robotToolBar->addAction("Emergency Stop");
    eStopBtn->setToolTip("Emergency Stop (Esc)");

    m_grindingToolBar = addToolBar("Grinding");
    m_grindingToolBar->setObjectName("GrindingToolBar");
    m_grindingToolBar->addAction("Generate Path");
    m_grindingToolBar->addAction("Simulate");
    m_grindingToolBar->addAction("Execute");
    m_grindingToolBar->addSeparator();
    m_grindingToolBar->addAction("Measure");
    m_grindingToolBar->addAction("Section");
}

// ============================================================================
// Central area placeholder
// ============================================================================
void MainWindow::setupCentralWidget()
{
    QWidget* placeholder = new QWidget(this);
    placeholder->setObjectName("CentralViewport");
    placeholder->setStyleSheet("background-color: #2b2b2b;");
    QVBoxLayout* layout = new QVBoxLayout(placeholder);
    QLabel* hint = new QLabel(tr("Loading 3D Viewer..."), placeholder);
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: #888; font-size: 18px;");
    layout->addWidget(hint);
    setCentralWidget(placeholder);
}

// ============================================================================
// Dock panels
// ============================================================================
void MainWindow::setupDockWidgets()
{
    m_modelBrowserDock = new QDockWidget("Model Browser", this);
    m_modelBrowserDock->setObjectName("ModelBrowserDock");
    m_modelBrowserDock->setWidget(new ModelBrowserPanel(this));
    m_modelBrowserDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_modelBrowserDock);

    m_propertyDock = new QDockWidget("Properties", this);
    m_propertyDock->setObjectName("PropertyDock");
    m_propertyDock->setWidget(new PropertyPanel(this));
    m_propertyDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_propertyDock);

    m_bottomDock = new QDockWidget("Output", this);
    m_bottomDock->setObjectName("BottomDock");
    m_bottomTabWidget = new QTabWidget(this);
    m_bottomTabWidget->addTab(new LogPanel(this),         "Log");
    m_bottomTabWidget->addTab(new RosMonitorPanel(this),  "ROS Topics");
    m_bottomTabWidget->addTab(new PathDataPanel(this),    "Path Data");
    m_bottomDock->setWidget(m_bottomTabWidget);
    addDockWidget(Qt::BottomDockWidgetArea, m_bottomDock);

    m_windowMenu->addAction(m_modelBrowserDock->toggleViewAction());
    m_windowMenu->addAction(m_propertyDock->toggleViewAction());
    m_windowMenu->addAction(m_bottomDock->toggleViewAction());
    m_windowMenu->addSeparator();
    m_windowMenu->addAction("Reset Layout");
}

// ============================================================================
// Status bar
// ============================================================================
void MainWindow::setupStatusBar()
{
    QStatusBar* sbar = statusBar();
    m_coordLabel = new QLabel("", this);
    m_coordLabel->setMinimumWidth(250);
    sbar->addWidget(m_coordLabel);

    m_rosStatusLabel = new QLabel("", this);
    m_rosStatusLabel->setMinimumWidth(150);
    sbar->addWidget(m_rosStatusLabel);

    m_modelInfoLabel = new QLabel("", this);
    sbar->addPermanentWidget(m_modelInfoLabel);
}

// ============================================================================
// Signal connections
// ============================================================================
void MainWindow::setupConnections()
{
    auto* bus  = EventBus::instance();
    auto* data = DataModel::instance();

    connect(data, &DataModel::modelLoaded, this, [this](const QString& path) {
        m_modelInfoLabel->setText(tr("Model: %1").arg(QFileInfo(path).fileName()));
    });

    connect(data, &DataModel::robotStateChanged, this, [this](const RobotState& state) {
        m_rosStatusLabel->setText(
            state.connected ? tr("ROS: Connected") : tr("ROS: Not Connected"));
    });

    connect(bus, &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            if (event == "viewer.cursor.moved") {
                m_coordLabel->setText(
                    tr("Coordinate: X:%1 Y:%2 Z:%3")
                        .arg(data.value("x").toDouble(), 0, 'f', 1)
                        .arg(data.value("y").toDouble(), 0, 'f', 1)
                        .arg(data.value("z").toDouble(), 0, 'f', 1));
            }
        });
}
