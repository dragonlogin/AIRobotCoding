#include <QApplication>
#include <QSurfaceFormat>

#include "MainWindow.h"
#include "PluginManager.h"
#include "modules/cad/CadModule.h"
#include "modules/viewer/ViewerModule.h"
#include "modules/robot/RobotModule.h"
#include "modules/pathplan/PathPlanModule.h"
#include "modules/grinding/GrindingModule.h"
#include "core/EventBus.h"

int main(int argc, char* argv[])
{
    // OpenGL 配置
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(4);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setApplicationName("AIRobot 曲面打磨系统");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("AIRobot");

    // 创建主窗口
    MainWindow mainWindow;

    // 初始化插件管理器
    PluginManager* pm = PluginManager::instance();
    pm->setMainWindow(&mainWindow);

    // 注册内置模块（按依赖顺序）
    pm->registerModule(new CadModule());
    pm->registerModule(new ViewerModule());
    pm->registerModule(new RobotModule());
    pm->registerModule(new PathPlanModule());
    pm->registerModule(new GrindingModule());

    // 初始化所有模块
    if (!pm->initializeAll()) {
        return -1;
    }

    // 将 Viewer 模块的 widget 设置为中心 widget
    auto* viewerMod = dynamic_cast<ViewerModule*>(pm->module("viewer"));
    if (viewerMod && viewerMod->viewerWidget()) {
        mainWindow.setCentralWidget(viewerMod->viewerWidget());
    }

    // 加载动态插件（如果有）
    pm->loadPlugins(app.applicationDirPath() + "/plugins");

    // 启动日志
    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "AIRobot 曲面打磨系统启动完成"}
    });

    mainWindow.show();
    return app.exec();
}
