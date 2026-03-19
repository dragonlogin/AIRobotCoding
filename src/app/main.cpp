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
    // OpenGL configuration
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CompatibilityProfile); // OSG depends on fixed pipeline; CoreProfile cannot be used
    format.setSamples(4);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setApplicationName("AIRobot Surface Grinding System");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("AIRobot");

    // Create main window
    MainWindow mainWindow;

    // Initialize plugin manager
    PluginManager* pm = PluginManager::instance();
    pm->setMainWindow(&mainWindow);

    // Register built-in modules (in dependency order)
    pm->registerModule(new CadModule());
    pm->registerModule(new ViewerModule());
    pm->registerModule(new RobotModule());
    pm->registerModule(new PathPlanModule());
    pm->registerModule(new GrindingModule());

    // Initialize all modules
    if (!pm->initializeAll()) {
        return -1;
    }

    // Set the Viewer module widget as the central widget
    auto* viewerMod = dynamic_cast<ViewerModule*>(pm->module("viewer"));
    if (viewerMod && viewerMod->viewerWidget()) {
        mainWindow.setCentralWidget(viewerMod->viewerWidget());
    }

    // Load dynamic plugins (if any)
    pm->loadPlugins(app.applicationDirPath() + "/plugins");

    // Startup log
    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "AIRobot Surface Grinding System started successfully"}
    });

    mainWindow.show();
    return app.exec();
}
