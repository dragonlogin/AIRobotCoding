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
#  define _USE_MATH_DEFINES  // MSVC 下 <cmath> 不默认导出 M_PI
#endif
#include <cmath>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("AIRobot 曲面打磨系统 v1.0");
    resize(1600, 900);
    setMinimumSize(1200, 700);

    menuBar()->setNativeMenuBar(false);  // macOS 下显示在窗口内而非系统菜单栏
    setupMenuBar();
    setupToolBars();
    setupCentralWidget();
    setupDockWidgets();
    setupStatusBar();
    setupConnections();
    loadStyleSheet();

    // 默认布局：左侧模型浏览器 250px，右侧属性面板 300px
    resizeDocks({m_modelBrowserDock}, {250}, Qt::Horizontal);
    resizeDocks({m_propertyDock}, {300}, Qt::Horizontal);
    resizeDocks({m_bottomDock}, {180}, Qt::Vertical);
}

MainWindow::~MainWindow()
{
    // 模块清理由 PluginManager 负责
}

void MainWindow::registerModule(IModule* module)
{
    if (!module || m_modules.contains(module->moduleId()))
        return;

    m_modules.insert(module->moduleId(), module);

    // 集成模块提供的 DockWidget
    for (QWidget* widget : module->dockWidgets()) {
        // 模块自行决定停靠位置，此处不强制
    }

    // 集成模块提供的工具栏 Action
    for (QAction* action : module->toolBarActions()) {
        m_grindingToolBar->addAction(action);
    }

    // 集成模块提供的菜单 Action
    for (QAction* action : module->menuActions()) {
        m_toolMenu->addAction(action);
    }
}

// ============================================================================
// 菜单栏
// ============================================================================
void MainWindow::setupMenuBar()
{
    QMenuBar* menuBar = this->menuBar();

    // -- 文件菜单 --
    m_fileMenu = menuBar->addMenu("文件(&F)");
    QAction* importAction = m_fileMenu->addAction("导入 STEP 模型(&I)...");
    importAction->setShortcut(QKeySequence("Ctrl+I"));
    m_fileMenu->addAction("导入机器人模型(&R)...");
    m_fileMenu->addSeparator();
    m_fileMenu->addAction("保存项目(&S)")->setShortcut(QKeySequence::Save);
    m_fileMenu->addAction("另存为(&A)...");
    m_fileMenu->addSeparator();
    m_fileMenu->addAction("导出打磨路径...");
    m_fileMenu->addAction("导出报告...");
    m_fileMenu->addSeparator();
    QAction* exitAction = m_fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // 导入 STEP 文件
    connect(importAction, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this, "导入 STEP 模型", "",
            "STEP 文件 (*.step *.stp);;所有文件 (*.*)");
        if (!path.isEmpty()) {
            EventBus::instance()->publish("cad.import.request", {{"path", path}});
        }
    });

    // -- 编辑菜单 --
    m_editMenu = menuBar->addMenu("编辑(&E)");
    m_editMenu->addAction("撤销(&U)")->setShortcut(QKeySequence::Undo);
    m_editMenu->addAction("重做(&R)")->setShortcut(QKeySequence::Redo);
    m_editMenu->addSeparator();
    m_editMenu->addAction("首选项(&P)...");

    // -- 视图菜单 --
    m_viewMenu = menuBar->addMenu("视图(&V)");
    m_viewMenu->addAction("前视图")->setShortcut(QKeySequence("1"));
    m_viewMenu->addAction("后视图")->setShortcut(QKeySequence("2"));
    m_viewMenu->addAction("左视图")->setShortcut(QKeySequence("3"));
    m_viewMenu->addAction("右视图")->setShortcut(QKeySequence("4"));
    m_viewMenu->addAction("顶视图")->setShortcut(QKeySequence("5"));
    m_viewMenu->addAction("底视图")->setShortcut(QKeySequence("6"));
    m_viewMenu->addAction("等轴测视图")->setShortcut(QKeySequence("0"));
    m_viewMenu->addSeparator();
    m_viewMenu->addAction("适应窗口(&F)")->setShortcut(QKeySequence("F"));
    m_viewMenu->addSeparator();
    QAction* wireframe = m_viewMenu->addAction("线框模式");
    wireframe->setCheckable(true);
    QAction* shaded = m_viewMenu->addAction("实体模式");
    shaded->setCheckable(true);
    shaded->setChecked(true);
    QAction* transparent = m_viewMenu->addAction("透明模式");
    transparent->setCheckable(true);

    // -- 工具菜单 --
    m_toolMenu = menuBar->addMenu("工具(&T)");
    m_toolMenu->addAction("测量距离");
    m_toolMenu->addAction("测量角度");
    m_toolMenu->addAction("截面分析");
    m_toolMenu->addAction("曲率分析");

    // -- 机器人菜单 --
    m_robotMenu = menuBar->addMenu("机器人(&R)");
    m_robotMenu->addAction("连接 ROS Master...");
    m_robotMenu->addAction("断开连接");
    m_robotMenu->addSeparator();
    m_robotMenu->addAction("加载机器人 URDF...");
    m_robotMenu->addAction("示教模式");
    m_robotMenu->addAction("手动控制");
    m_robotMenu->addSeparator();
    QAction* eStop = m_robotMenu->addAction("急停 (&E)");
    eStop->setShortcut(QKeySequence("Escape"));

    // -- 路径菜单 --
    m_pathMenu = menuBar->addMenu("路径(&P)");
    m_pathMenu->addAction("生成打磨路径...");
    m_pathMenu->addAction("路径优化...");
    m_pathMenu->addAction("碰撞检测");
    m_pathMenu->addSeparator();
    m_pathMenu->addAction("仿真运行(&S)")->setShortcut(QKeySequence("F5"));
    m_pathMenu->addAction("停止仿真")->setShortcut(QKeySequence("Shift+F5"));
    m_pathMenu->addSeparator();
    m_pathMenu->addAction("执行打磨(&X)")->setShortcut(QKeySequence("F6"));

    // -- 窗口菜单 --
    m_windowMenu = menuBar->addMenu("窗口(&W)");
    // dock widget 的显示/隐藏 action 在 setupDockWidgets 中添加

    // -- 库菜单 --
    m_libraryMenu = menuBar->addMenu("库(&L)");
    setupLibraryMenus(m_libraryMenu);

    // -- 帮助菜单 --
    m_helpMenu = menuBar->addMenu("帮助(&H)");
    m_helpMenu->addAction("使用手册");
    m_helpMenu->addAction("关于(&A)...");
}

// ============================================================================
// 机器人库 & STEP 工件库
// ============================================================================
void MainWindow::setupLibraryMenus(QMenu* libraryMenu)
{
    // ── 机器人库 ──
    static const RobotLibEntry robotLib[] = {
        { "ur5",     "UR5",          "Universal Robots", 6, 850,  5,  { 0, -90, 90, -90,  0, 0 } },
        { "ur10",    "UR10",         "Universal Robots", 6, 1300, 10, { 0, -90, 90, -90,  0, 0 } },
        { "kr6_r900","KR6 R900",     "KUKA",             6, 900,  6,  { 0,  45, 90,   0, 90, 0 } },
        { "irb1200", "IRB 1200",     "ABB",              6, 700,  5,  { 0,  30, 60,   0, 60, 0 } },
        { "m10ia",   "M10iA",        "Fanuc",            6, 1422, 10, { 0, -60, 80,   0, 70, 0 } },
        { "cr7ia",   "CR-7iA (协作)","Fanuc",            6, 717,  7,  { 0, -45, 90,   0, 45, 0 } },
    };

    QMenu* robotLibMenu = libraryMenu->addMenu("机器人库");
    for (const auto& entry : robotLib) {
        QString label = QString("%1  [%2]  到达 %3mm / %4kg")
                            .arg(entry.name, entry.manufacturer)
                            .arg(entry.reach).arg(entry.payload);
        QAction* act = robotLibMenu->addAction(label);
        connect(act, &QAction::triggered, this, [this, entry]() {
            loadRobotFromLibrary(entry);
        });
    }

    libraryMenu->addSeparator();

    // ── 机器人 STEP 模型库（外观/仿真用）──
    robotLibMenu->addSeparator();
    QAction* kukaStepAct = robotLibMenu->addAction("KUKA KR600 R2830  [三维模型]  导入 STEP...");
    connect(kukaStepAct, &QAction::triggered, this, [this]() {
        QString baseDir  = robotsResourceDir();
        QString filePath = QDir(baseDir).filePath("KR600_R2830.stp");
        if (!QFile::exists(filePath))
            filePath = QFileDialog::getOpenFileName(this, "加载机器人 STEP 模型",
                            baseDir, "STEP 文件 (*.step *.stp)");
        if (!filePath.isEmpty())
            EventBus::instance()->publish("cad.import.request", {{"path", filePath}});
    });

    libraryMenu->addSeparator();

    // ── STEP 工件库 ──
    static const WorkpieceLibEntry workpieceLib[] = {
        { "多特征机械零件",  "含平面/孔/槽，AP214 标准测试件",  "flat_plate.stp"      },
        { "圆柱装配件",      "多零件装配，含圆柱曲面",          "cylinder.stp"        },
        { "B样条笼形曲面",   "样条曲面结构，适合路径规划验证",  "sphere_half.stp"     },
        { "通风叶轮",        "复杂旋转曲面，类涡轮叶片结构",    "turbine_blade.stp"   },
        { "多面识别零件",    "多种曲面类型，适合曲面识别测试",  "freeform_surface.stp"},
        { "复杂装配体",      "大型自由曲面装配，高面数测试",    "complex_surface.stp" },
        { "标准装配测试件",  "AP214 装配标准测试件",            "assembly_test.stp"   },
    };

    QMenu* workpieceLibMenu = libraryMenu->addMenu("STEP 工件库");
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
    state.statusText = QString("[库] %1 (%2) 已加载").arg(entry.name, entry.manufacturer);
    for (int i = 0; i < 6; ++i)
        state.joints[i] = entry.joints[i];
    // 简单正运动学近似：末端 Z = reach * sin(joint[1] deg)
    double j1r = entry.joints[1] * M_PI / 180.0;
    state.tcpPosition = QVector3D(
        static_cast<float>(entry.reach * 0.6),
        0.0f,
        static_cast<float>(entry.reach * std::abs(std::sin(j1r))));

    DataModel::instance()->updateRobotState(state);

    m_rosStatusLabel->setText(QString("机器人: %1").arg(entry.name));
    EventBus::instance()->publish("log.message", {
        {"level",   "INFO"},
        {"message", QString("已从库加载机器人: %1 | 厂商: %2 | 自由度: %3 | 到达范围: %4 mm")
             .arg(entry.name, entry.manufacturer)
             .arg(entry.dof).arg(entry.reach)}
    });
}

// 跨平台资源目录定位：
//   macOS/Linux: 可执行文件在 build/bin/，resources 在源码根目录
//   Windows MSVC: 可执行文件在 build/Debug 或 build/Release 下
static QString findResourceDir(const QString& subDir)
{
    const QString exe = QCoreApplication::applicationDirPath();
    // 依次尝试常见的相对位置
    for (const QString& rel : { "../resources/" + subDir,
                                 "../../resources/" + subDir,
                                 "../../../resources/" + subDir,
                                 "resources/" + subDir }) {
        QString candidate = QDir(exe).filePath(rel);
        if (QDir(candidate).exists())
            return QDir(candidate).absolutePath();
    }
    return QDir(exe).filePath("../resources/" + subDir); // 兜底
}

static QString robotsResourceDir()    { return findResourceDir("robots");     }
static QString workpiecesResourceDir(){ return findResourceDir("workpieces"); }

void MainWindow::loadWorkpieceFromLibrary(const WorkpieceLibEntry& entry)
{
    QString baseDir  = workpiecesResourceDir();
    QString filePath = QDir(baseDir).filePath(entry.fileName);

    if (!QFile::exists(filePath)) {
        // 让用户手动选择
        filePath = QFileDialog::getOpenFileName(
            this,
            QString("导入工件: %1").arg(entry.name),
            baseDir,
            "STEP 文件 (*.step *.stp);;所有文件 (*.*)");
    }

    if (filePath.isEmpty())
        return;

    EventBus::instance()->publish("cad.import.request", {{"path", filePath}});
    EventBus::instance()->publish("log.message", {
        {"level",   "INFO"},
        {"message", QString("已从库加载工件: %1 — %2").arg(entry.name, entry.description)}
    });
}

// ============================================================================
// 工具栏
// ============================================================================
void MainWindow::setupToolBars()
{
    // 文件工具栏
    m_fileToolBar = addToolBar("文件");
    m_fileToolBar->setObjectName("FileToolBar");
    m_fileToolBar->setIconSize(QSize(24, 24));
    m_fileToolBar->addAction("导入");
    m_fileToolBar->addAction("保存");
    m_fileToolBar->addSeparator();
    m_fileToolBar->addAction("撤销");
    m_fileToolBar->addAction("重做");

    // 选择工具栏
    m_selectionToolBar = addToolBar("选择");
    m_selectionToolBar->setObjectName("SelectionToolBar");
    m_selectionToolBar->addAction("选择面");
    m_selectionToolBar->addAction("选择边");
    m_selectionToolBar->addAction("选择体");
    m_selectionToolBar->addAction("框选");

    // 机器人工具栏
    m_robotToolBar = addToolBar("机器人");
    m_robotToolBar->setObjectName("RobotToolBar");
    m_robotToolBar->addAction("连接");
    QAction* eStopBtn = m_robotToolBar->addAction("急停");
    eStopBtn->setToolTip("紧急停止 (Esc)");

    // 打磨工具栏
    m_grindingToolBar = addToolBar("打磨");
    m_grindingToolBar->setObjectName("GrindingToolBar");
    m_grindingToolBar->addAction("生成路径");
    m_grindingToolBar->addAction("仿真");
    m_grindingToolBar->addAction("执行");
    m_grindingToolBar->addSeparator();
    m_grindingToolBar->addAction("测量");
    m_grindingToolBar->addAction("截面");
}

// ============================================================================
// 中心区域 - 3D 视图占位（由 Viewer 模块提供实际 widget）
// ============================================================================
void MainWindow::setupCentralWidget()
{
    // 3D 视图作为中心 widget，实际由 ViewerModule 创建
    // 这里先放置占位 widget，模块初始化时替换
    QWidget* placeholder = new QWidget(this);
    placeholder->setObjectName("CentralViewport");
    placeholder->setStyleSheet("background-color: #2b2b2b;");

    QVBoxLayout* layout = new QVBoxLayout(placeholder);
    QLabel* hint = new QLabel("3D 视图加载中...", placeholder);
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: #888; font-size: 18px;");
    layout->addWidget(hint);

    setCentralWidget(placeholder);
}

// ============================================================================
// 停靠面板
// ============================================================================
void MainWindow::setupDockWidgets()
{
    // -- 左侧：模型浏览器 --
    m_modelBrowserDock = new QDockWidget("模型浏览器", this);
    m_modelBrowserDock->setObjectName("ModelBrowserDock");
    m_modelBrowserDock->setWidget(new ModelBrowserPanel(this));
    m_modelBrowserDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_modelBrowserDock);

    // -- 右侧：属性面板 --
    m_propertyDock = new QDockWidget("属性", this);
    m_propertyDock->setObjectName("PropertyDock");
    m_propertyDock->setWidget(new PropertyPanel(this));
    m_propertyDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_propertyDock);

    // -- 底部：日志/ROS/路径数据 标签页 --
    m_bottomDock = new QDockWidget("输出", this);
    m_bottomDock->setObjectName("BottomDock");
    m_bottomTabWidget = new QTabWidget(this);
    m_bottomTabWidget->addTab(new LogPanel(this), "日志");
    m_bottomTabWidget->addTab(new RosMonitorPanel(this), "ROS 话题");
    m_bottomTabWidget->addTab(new PathDataPanel(this), "路径数据");
    m_bottomDock->setWidget(m_bottomTabWidget);
    addDockWidget(Qt::BottomDockWidgetArea, m_bottomDock);

    // 添加到窗口菜单
    m_windowMenu->addAction(m_modelBrowserDock->toggleViewAction());
    m_windowMenu->addAction(m_propertyDock->toggleViewAction());
    m_windowMenu->addAction(m_bottomDock->toggleViewAction());
    m_windowMenu->addSeparator();
    m_windowMenu->addAction("重置布局");
}

// ============================================================================
// 状态栏
// ============================================================================
void MainWindow::setupStatusBar()
{
    QStatusBar* sbar = statusBar();

    m_coordLabel = new QLabel("坐标: X:0.0 Y:0.0 Z:0.0", this);
    m_coordLabel->setMinimumWidth(250);
    sbar->addWidget(m_coordLabel);

    m_rosStatusLabel = new QLabel("ROS: 未连接", this);
    m_rosStatusLabel->setMinimumWidth(150);
    sbar->addWidget(m_rosStatusLabel);

    m_modelInfoLabel = new QLabel("模型: 无", this);
    sbar->addPermanentWidget(m_modelInfoLabel);
}

// ============================================================================
// 信号连接
// ============================================================================
void MainWindow::setupConnections()
{
    auto* bus = EventBus::instance();
    auto* data = DataModel::instance();

    // 模型加载完成 -> 更新状态栏
    connect(data, &DataModel::modelLoaded, this, [this](const QString& path) {
        m_modelInfoLabel->setText(QString("模型: %1").arg(QFileInfo(path).fileName()));
    });

    // 机器人状态变化 -> 更新状态栏
    connect(data, &DataModel::robotStateChanged, this, [this](const RobotState& state) {
        m_rosStatusLabel->setText(
            state.connected ? "ROS: 已连接" : "ROS: 未连接");
    });

    // 3D 视图坐标更新
    connect(bus, &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            if (event == "viewer.cursor.moved") {
                double x = data.value("x").toDouble();
                double y = data.value("y").toDouble();
                double z = data.value("z").toDouble();
                m_coordLabel->setText(
                    QString("坐标: X:%1 Y:%2 Z:%3")
                        .arg(x, 0, 'f', 1)
                        .arg(y, 0, 'f', 1)
                        .arg(z, 0, 'f', 1));
            }
        });
}

// ============================================================================
// 样式表
// ============================================================================
void MainWindow::loadStyleSheet()
{
    QFile styleFile(":/styles/dark_theme.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(styleFile.readAll());
        styleFile.close();
    }
}
