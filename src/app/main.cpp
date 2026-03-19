#include <QApplication>
#include <QSurfaceFormat>
#include <QTranslator>
#include <QLocale>
#include <QSettings>

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
    // High-DPI support — must be set before QApplication is constructed
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // OpenGL configuration
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    format.setSamples(4);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setApplicationName("AIRobot Surface Grinding System");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("AIRobot");

    // Restore language from settings and install translator
    QSettings settings("AIRobot", "AIRobotGrinding");
    QString lang = settings.value("language", "en").toString();
    QTranslator* translator = new QTranslator(&app);
    if (lang == "zh") {
        if (translator->load("zh_CN", app.applicationDirPath() + "/translations")) {
            app.installTranslator(translator);
        }
    }

    // Create main window
    MainWindow mainWindow;

    // Initialize plugin manager
    PluginManager* pm = PluginManager::instance();
    pm->setMainWindow(&mainWindow);

    pm->registerModule(new CadModule());
    pm->registerModule(new ViewerModule());
    pm->registerModule(new RobotModule());
    pm->registerModule(new PathPlanModule());
    pm->registerModule(new GrindingModule());

    if (!pm->initializeAll()) {
        return -1;
    }

    auto* viewerMod = dynamic_cast<ViewerModule*>(pm->module("viewer"));
    if (viewerMod && viewerMod->viewerWidget()) {
        mainWindow.setCentralWidget(viewerMod->viewerWidget());
    }

    pm->loadPlugins(app.applicationDirPath() + "/plugins");

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "AIRobot Surface Grinding System started successfully"}
    });

    mainWindow.show();
    return app.exec();
}
