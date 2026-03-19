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
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QDir>
#include <QStandardPaths>
#ifdef _WIN32
#  define _USE_MATH_DEFINES  // MSVC does not export M_PI from <cmath> by default
#endif
#include <cmath>

// Forward declarations for file-local helpers defined later in this file
static QString findResourceDir(const QString& subDir);
static QString robotsResourceDir();
static QString workpiecesResourceDir();

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("AIRobot Surface Grinding System v1.0");
    resize(1600, 900);
    setMinimumSize(1200, 700);

    menuBar()->setNativeMenuBar(false);  // On macOS, show menu bar inside the window rather than the system menu bar
    setupMenuBar();
    setupToolBars();
    setupCentralWidget();
    setupDockWidgets();
    setupStatusBar();
    setupConnections();
    loadStyleSheet();

    // Default layout: left model browser 250px, right property panel 300px
    resizeDocks({m_modelBrowserDock}, {250}, Qt::Horizontal);
    resizeDocks({m_propertyDock}, {300}, Qt::Horizontal);
    resizeDocks({m_bottomDock}, {180}, Qt::Vertical);
}

MainWindow::~MainWindow()
{
    // Module cleanup is handled by PluginManager
}

void MainWindow::registerModule(IModule* module)
{
    if (!module || m_modules.contains(module->moduleId()))
        return;

    m_modules.insert(module->moduleId(), module);

    // Integrate dock widgets provided by the module
    for (QWidget* widget : module->dockWidgets()) {
        // The module decides its own dock position; nothing is forced here
    }

    // Integrate toolbar actions provided by the module
    for (QAction* action : module->toolBarActions()) {
        m_grindingToolBar->addAction(action);
    }

    // Integrate menu actions provided by the module
    for (QAction* action : module->menuActions()) {
        m_toolMenu->addAction(action);
    }
}

// ============================================================================
// Menu bar
// ============================================================================
void MainWindow::setupMenuBar()
{
    QMenuBar* menuBar = this->menuBar();

    // -- File menu --
    m_fileMenu = menuBar->addMenu("File(&F)");
    QAction* importAction = m_fileMenu->addAction("Import STEP Model(&I)...");
    importAction->setShortcut(QKeySequence("Ctrl+I"));
    m_fileMenu->addAction("Import Robot Model(&R)...");
    m_fileMenu->addSeparator();
    m_fileMenu->addAction("Save Project(&S)")->setShortcut(QKeySequence::Save);
    m_fileMenu->addAction("Save As(&A)...");
    m_fileMenu->addSeparator();
    m_fileMenu->addAction("Export Grinding Path...");
    m_fileMenu->addAction("Export Report...");
    m_fileMenu->addSeparator();
    QAction* exitAction = m_fileMenu->addAction("Exit(&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // Import STEP file
    connect(importAction, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this, "Import STEP Model", "",
            "STEP Files (*.step *.stp);;All Files (*.*)");
        if (!path.isEmpty()) {
            EventBus::instance()->publish("cad.import.request", {{"path", path}});
        }
    });

    // -- Edit menu --
    m_editMenu = menuBar->addMenu("Edit(&E)");
    m_editMenu->addAction("Undo(&U)")->setShortcut(QKeySequence::Undo);
    m_editMenu->addAction("Redo(&R)")->setShortcut(QKeySequence::Redo);
    m_editMenu->addSeparator();
    m_editMenu->addAction("Preferences(&P)...");

    // -- View menu --
    m_viewMenu = menuBar->addMenu("View(&V)");
    m_viewMenu->addAction("Front View")->setShortcut(QKeySequence("1"));
    m_viewMenu->addAction("Back View")->setShortcut(QKeySequence("2"));
    m_viewMenu->addAction("Left View")->setShortcut(QKeySequence("3"));
    m_viewMenu->addAction("Right View")->setShortcut(QKeySequence("4"));
    m_viewMenu->addAction("Top View")->setShortcut(QKeySequence("5"));
    m_viewMenu->addAction("Bottom View")->setShortcut(QKeySequence("6"));
    m_viewMenu->addAction("Isometric View")->setShortcut(QKeySequence("0"));
    m_viewMenu->addSeparator();
    m_viewMenu->addAction("Fit to Window(&F)")->setShortcut(QKeySequence("F"));
    m_viewMenu->addSeparator();
    QAction* wireframe = m_viewMenu->addAction("Wireframe Mode");
    wireframe->setCheckable(true);
    QAction* shaded = m_viewMenu->addAction("Shaded Mode");
    shaded->setCheckable(true);
    shaded->setChecked(true);
    QAction* transparent = m_viewMenu->addAction("Transparent Mode");
    transparent->setCheckable(true);

    // -- Tools menu --
    m_toolMenu = menuBar->addMenu("Tools(&T)");
    m_toolMenu->addAction("Measure Distance");
    m_toolMenu->addAction("Measure Angle");
    m_toolMenu->addAction("Section Analysis");
    m_toolMenu->addAction("Curvature Analysis");

    // -- Robot menu --
    m_robotMenu = menuBar->addMenu("Robot(&R)");
    m_robotMenu->addAction("Connect to ROS Master...");
    m_robotMenu->addAction("Disconnect");
    m_robotMenu->addSeparator();
    m_robotMenu->addAction("Load Robot URDF...");
    m_robotMenu->addAction("Teaching Mode");
    m_robotMenu->addAction("Manual Control");
    m_robotMenu->addSeparator();
    QAction* eStop = m_robotMenu->addAction("Emergency Stop(&E)");
    eStop->setShortcut(QKeySequence("Escape"));

    // -- Path menu --
    m_pathMenu = menuBar->addMenu("Path(&P)");
    m_pathMenu->addAction("Generate Grinding Path...");
    m_pathMenu->addAction("Path Optimization...");
    m_pathMenu->addAction("Collision Detection");
    m_pathMenu->addSeparator();
    m_pathMenu->addAction("Run Simulation(&S)")->setShortcut(QKeySequence("F5"));
    m_pathMenu->addAction("Stop Simulation")->setShortcut(QKeySequence("Shift+F5"));
    m_pathMenu->addSeparator();
    m_pathMenu->addAction("Execute Grinding(&X)")->setShortcut(QKeySequence("F6"));

    // -- Window menu --
    m_windowMenu = menuBar->addMenu("Window(&W)");
    // Dock widget show/hide actions are added in setupDockWidgets

    // -- Library menu --
    m_libraryMenu = menuBar->addMenu("Library(&L)");
    setupLibraryMenus(m_libraryMenu);

    // -- Help menu --
    m_helpMenu = menuBar->addMenu("Help(&H)");
    m_helpMenu->addAction("User Manual");
    m_helpMenu->addAction("About(&A)...");
}

// ============================================================================
// Robot library & STEP workpiece library
// ============================================================================
void MainWindow::setupLibraryMenus(QMenu* libraryMenu)
{
    // ── Robot library ──
    static const RobotLibEntry robotLib[] = {
        { "ur5",     "UR5",                    "Universal Robots", 6, 850,  5,  { 0, -90, 90, -90,  0, 0 } },
        { "ur10",    "UR10",                   "Universal Robots", 6, 1300, 10, { 0, -90, 90, -90,  0, 0 } },
        { "kr6_r900","KR6 R900",               "KUKA",             6, 900,  6,  { 0,  45, 90,   0, 90, 0 } },
        { "irb1200", "IRB 1200",               "ABB",              6, 700,  5,  { 0,  30, 60,   0, 60, 0 } },
        { "m10ia",   "M10iA",                  "Fanuc",            6, 1422, 10, { 0, -60, 80,   0, 70, 0 } },
        { "cr7ia",   "CR-7iA (Collaborative)", "Fanuc",            6, 717,  7,  { 0, -45, 90,   0, 45, 0 } },
    };

    QMenu* robotLibMenu = libraryMenu->addMenu("Robot Library");
    for (const auto& entry : robotLib) {
        QString label = QString("%1  [%2]  Reach %3mm / %4kg")
                            .arg(entry.name, entry.manufacturer)
                            .arg(entry.reach).arg(entry.payload);
        QAction* act = robotLibMenu->addAction(label);
        connect(act, &QAction::triggered, this, [this, entry]() {
            loadRobotFromLibrary(entry);
        });
    }

    libraryMenu->addSeparator();

    // ── Robot STEP model library (for appearance / simulation) ──
    robotLibMenu->addSeparator();
    QAction* kukaStepAct = robotLibMenu->addAction("KUKA KR600 R2830  [3D Model]  Import STEP...");
    connect(kukaStepAct, &QAction::triggered, this, [this]() {
        QString baseDir  = robotsResourceDir();
        QString filePath = QDir(baseDir).filePath("KR600_R2830.stp");
        if (!QFile::exists(filePath))
            filePath = QFileDialog::getOpenFileName(this, "Load Robot STEP Model",
                            baseDir, "STEP Files (*.step *.stp)");
        if (!filePath.isEmpty())
            EventBus::instance()->publish("cad.import.request", {{"path", filePath}});
    });

    libraryMenu->addSeparator();

    // ── STEP workpiece library ──
    static const WorkpieceLibEntry workpieceLib[] = {
        { "Multi-Feature Mechanical Part",  "Planes/holes/slots, AP214 standard test piece",     "flat_plate.stp"      },
        { "Cylindrical Assembly",           "Multi-part assembly with cylindrical surfaces",      "cylinder.stp"        },
        { "B-Spline Cage Surface",          "Spline surface structure, suitable for path planning verification", "sphere_half.stp" },
        { "Ventilation Impeller",           "Complex rotational surface, turbine blade-like structure", "turbine_blade.stp" },
        { "Multi-Face Recognition Part",    "Multiple surface types, suitable for surface recognition testing", "freeform_surface.stp" },
        { "Complex Assembly",               "Large freeform surface assembly, high face count test", "complex_surface.stp" },
        { "Standard Assembly Test Piece",   "AP214 assembly standard test piece",                "assembly_test.stp"   },
    };

    QMenu* workpieceLibMenu = libraryMenu->addMenu("STEP Workpiece Library");
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
    // Simple FK approximation: TCP Z = reach * sin(joint[1] deg)
    double j1r = entry.joints[1] * M_PI / 180.0;
    state.tcpPosition = QVector3D(
        static_cast<float>(entry.reach * 0.6),
        0.0f,
        static_cast<float>(entry.reach * std::abs(std::sin(j1r))));

    DataModel::instance()->updateRobotState(state);

    m_rosStatusLabel->setText(QString("Robot: %1").arg(entry.name));
    EventBus::instance()->publish("log.message", {
        {"level",   "INFO"},
        {"message", QString("Robot loaded from library: %1 | Manufacturer: %2 | DOF: %3 | Reach: %4 mm")
             .arg(entry.name, entry.manufacturer)
             .arg(entry.dof).arg(entry.reach)}
    });
}

// Cross-platform resource directory lookup:
//   macOS/Linux: executable is in build/bin/, resources are in the source root
//   Windows MSVC: executable is in build/Debug or build/Release
static QString findResourceDir(const QString& subDir)
{
    const QString exe = QCoreApplication::applicationDirPath();
    // Try common relative locations in order
    for (const QString& rel : { "../resources/" + subDir,
                                 "../../resources/" + subDir,
                                 "../../../resources/" + subDir,
                                 "resources/" + subDir }) {
        QString candidate = QDir(exe).filePath(rel);
        if (QDir(candidate).exists())
            return QDir(candidate).absolutePath();
    }
    return QDir(exe).filePath("../resources/" + subDir); // fallback
}

static QString robotsResourceDir()    { return findResourceDir("robots");     }
static QString workpiecesResourceDir(){ return findResourceDir("workpieces"); }

void MainWindow::loadWorkpieceFromLibrary(const WorkpieceLibEntry& entry)
{
    QString baseDir  = workpiecesResourceDir();
    QString filePath = QDir(baseDir).filePath(entry.fileName);

    if (!QFile::exists(filePath)) {
        // Let the user select manually
        filePath = QFileDialog::getOpenFileName(
            this,
            QString("Import Workpiece: %1").arg(entry.name),
            baseDir,
            "STEP Files (*.step *.stp);;All Files (*.*)");
    }

    if (filePath.isEmpty())
        return;

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
    // File toolbar
    m_fileToolBar = addToolBar("File");
    m_fileToolBar->setObjectName("FileToolBar");
    m_fileToolBar->setIconSize(QSize(24, 24));
    m_fileToolBar->addAction("Import");
    m_fileToolBar->addAction("Save");
    m_fileToolBar->addSeparator();
    m_fileToolBar->addAction("Undo");
    m_fileToolBar->addAction("Redo");

    // Selection toolbar
    m_selectionToolBar = addToolBar("Selection");
    m_selectionToolBar->setObjectName("SelectionToolBar");
    m_selectionToolBar->addAction("Select Face");
    m_selectionToolBar->addAction("Select Edge");
    m_selectionToolBar->addAction("Select Solid");
    m_selectionToolBar->addAction("Box Select");

    // Robot toolbar
    m_robotToolBar = addToolBar("Robot");
    m_robotToolBar->setObjectName("RobotToolBar");
    m_robotToolBar->addAction("Connect");
    QAction* eStopBtn = m_robotToolBar->addAction("Emergency Stop");
    eStopBtn->setToolTip("Emergency Stop (Esc)");

    // Grinding toolbar
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
// Central area - 3D view placeholder (actual widget provided by Viewer module)
// ============================================================================
void MainWindow::setupCentralWidget()
{
    // The 3D view is the central widget; it is actually created by ViewerModule.
    // A placeholder widget is placed here and replaced when the module initializes.
    QWidget* placeholder = new QWidget(this);
    placeholder->setObjectName("CentralViewport");
    placeholder->setStyleSheet("background-color: #2b2b2b;");

    QVBoxLayout* layout = new QVBoxLayout(placeholder);
    QLabel* hint = new QLabel("Loading 3D Viewer...", placeholder);
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
    // -- Left: Model browser --
    m_modelBrowserDock = new QDockWidget("Model Browser", this);
    m_modelBrowserDock->setObjectName("ModelBrowserDock");
    m_modelBrowserDock->setWidget(new ModelBrowserPanel(this));
    m_modelBrowserDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_modelBrowserDock);

    // -- Right: Property panel --
    m_propertyDock = new QDockWidget("Properties", this);
    m_propertyDock->setObjectName("PropertyDock");
    m_propertyDock->setWidget(new PropertyPanel(this));
    m_propertyDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_propertyDock);

    // -- Bottom: Log / ROS / Path data tabs --
    m_bottomDock = new QDockWidget("Output", this);
    m_bottomDock->setObjectName("BottomDock");
    m_bottomTabWidget = new QTabWidget(this);
    m_bottomTabWidget->addTab(new LogPanel(this), "Log");
    m_bottomTabWidget->addTab(new RosMonitorPanel(this), "ROS Topics");
    m_bottomTabWidget->addTab(new PathDataPanel(this), "Path Data");
    m_bottomDock->setWidget(m_bottomTabWidget);
    addDockWidget(Qt::BottomDockWidgetArea, m_bottomDock);

    // Add to window menu
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

    m_coordLabel = new QLabel("Coordinate: X:0.0 Y:0.0 Z:0.0", this);
    m_coordLabel->setMinimumWidth(250);
    sbar->addWidget(m_coordLabel);

    m_rosStatusLabel = new QLabel("ROS: Not Connected", this);
    m_rosStatusLabel->setMinimumWidth(150);
    sbar->addWidget(m_rosStatusLabel);

    m_modelInfoLabel = new QLabel("Model: None", this);
    sbar->addPermanentWidget(m_modelInfoLabel);
}

// ============================================================================
// Signal connections
// ============================================================================
void MainWindow::setupConnections()
{
    auto* bus = EventBus::instance();
    auto* data = DataModel::instance();

    // Model loaded -> update status bar
    connect(data, &DataModel::modelLoaded, this, [this](const QString& path) {
        m_modelInfoLabel->setText(QString("Model: %1").arg(QFileInfo(path).fileName()));
    });

    // Robot state changed -> update status bar
    connect(data, &DataModel::robotStateChanged, this, [this](const RobotState& state) {
        m_rosStatusLabel->setText(
            state.connected ? "ROS: Connected" : "ROS: Not Connected");
    });

    // 3D viewer coordinate update
    connect(bus, &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            if (event == "viewer.cursor.moved") {
                double x = data.value("x").toDouble();
                double y = data.value("y").toDouble();
                double z = data.value("z").toDouble();
                m_coordLabel->setText(
                    QString("Coordinate: X:%1 Y:%2 Z:%3")
                        .arg(x, 0, 'f', 1)
                        .arg(y, 0, 'f', 1)
                        .arg(z, 0, 'f', 1));
            }
        });
}

// ============================================================================
// Style sheet
// ============================================================================
void MainWindow::loadStyleSheet()
{
    QFile styleFile(":/styles/dark_theme.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(styleFile.readAll());
        styleFile.close();
    }
}
