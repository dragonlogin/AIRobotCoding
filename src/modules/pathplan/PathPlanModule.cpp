#include "PathPlanModule.h"
#include "core/EventBus.h"
#include "core/DataModel.h"
#include "modules/kinematics/KdlKinematics.h"

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>

#include <QAction>
#include <QElapsedTimer>

PathPlanModule::PathPlanModule(QObject* parent)
    : QObject(parent)
{
}

bool PathPlanModule::initialize()
{
    m_simulator = new PathSimulator(this);

    // Simulation finished signal
    connect(m_simulator, &PathSimulator::finished, this, []() {
        EventBus::instance()->publish("log.message", {
            {"level", "INFO"},
            {"message", "Path simulation finished"}
        });
    });

    // Listen for events
    connect(EventBus::instance(), &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            Q_UNUSED(data)
            if (event == "pathplan.generate.request") {
                generatePath();
            } else if (event == "pathplan.optimize.request") {
                optimizePath();
            } else if (event == "pathplan.simulate.start") {
                startSimulation();
            } else if (event == "pathplan.simulate.stop") {
                stopSimulation();
            }
        });

    return true;
}

void PathPlanModule::shutdown()
{
    m_simulator->stop();
}

QList<QAction*> PathPlanModule::menuActions()
{
    QList<QAction*> actions;

    QAction* genAction = new QAction("Generate Grinding Path", this);
    connect(genAction, &QAction::triggered, this, &PathPlanModule::generatePath);
    actions.append(genAction);

    QAction* optAction = new QAction("Optimize Path", this);
    connect(optAction, &QAction::triggered, this, &PathPlanModule::optimizePath);
    actions.append(optAction);

    QAction* simAction = new QAction("Run Simulation", this);
    simAction->setShortcut(QKeySequence("F5"));
    connect(simAction, &QAction::triggered, this, &PathPlanModule::startSimulation);
    actions.append(simAction);

    QAction* stopSimAction = new QAction("Stop Simulation", this);
    stopSimAction->setShortcut(QKeySequence("Shift+F5"));
    connect(stopSimAction, &QAction::triggered, this, &PathPlanModule::stopSimulation);
    actions.append(stopSimAction);

    return actions;
}

void PathPlanModule::generatePath()
{
    auto* bus = EventBus::instance();
    auto* data = DataModel::instance();

    if (data->tasks().isEmpty()) {
        bus->publish("log.message", {
            {"level", "WARN"},
            {"message", "Please create a grinding task and select a surface first"}
        });
        return;
    }

    GrindingTask& task = data->currentTask();

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("Starting grinding path generation [%1], strategy: Zigzag").arg(task.name)}
    });

    QElapsedTimer timer;
    timer.start();

    // Configure path generation parameters
    GrindingPathGenerator::Parameters genParams;
    genParams.strategy = GrindingPathGenerator::Zigzag;
    genParams.stepOver = task.stepOver;
    genParams.feedRate = task.feedRate;
    genParams.pressure = task.pressure;
    genParams.pointDensity = 100;
    genParams.smoothPath = true;
    m_generator.setParameters(genParams);

    // Generate and merge paths for each selected face
    QVector<PathPoint> allPaths;

    // Request face data from the event bus (handled by CadModule)
    // In the actual implementation, obtain a CadModule reference via PluginManager
    // Here we use the face indices stored in DataModel
    bus->publish("cad.faces.request", {
        {"faceIndices", QVariant::fromValue(task.selectedFaces)}
    });

    // Generate path (simplified here; actual implementation requires TopoDS_Face)
    // Check whether any faces are selected
    if (task.selectedFaces.isEmpty()) {
        // If no faces explicitly selected, generate for all faces
        bus->publish("log.message", {
            {"level", "INFO"},
            {"message", "No specific surfaces selected; generating path for all surfaces"}
        });
    }

    auto path = m_generator.generate();

    // Path optimization
    PathOptimizer::Parameters optParams;
    optParams.maxFeedRate = task.feedRate;
    optParams.basePressure = task.pressure;
    m_optimizer.setParameters(optParams);

    if (!path.isEmpty()) {
        path = m_optimizer.optimize(path);
    }

    task.path = path;

    auto stats = m_generator.statistics();
    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("Path generation complete: %1 points, total length %2 mm, estimated time %3 s, elapsed %4 ms")
                        .arg(stats.totalPoints)
                        .arg(stats.totalLength, 0, 'f', 1)
                        .arg(stats.estimatedTime, 0, 'f', 1)
                        .arg(timer.elapsed())}
    });

    // Notify the Viewer to refresh the path display
    bus->publish("pathplan.path.generated", {
        {"taskName", task.name},
        {"pointCount", stats.totalPoints}
    });

    emit data->tasksChanged();

    // Automatically compute IK after path generation
    computeIK();
}

void PathPlanModule::computeIK()
{
    auto* data = DataModel::instance();
    auto* bus  = EventBus::instance();
    if (data->tasks().isEmpty()) return;

    GrindingTask& task = data->currentTask();
    if (task.path.isEmpty()) return;

    // Lazy-load the kinematics solver
    if (!m_kinematics || m_kinematics->robotName() != task.robotType) {
        m_kinematics = KinematicsFactory::create(task.robotType);
        if (!m_kinematics) {
            bus->publish("log.message", {
                {"level", "WARN"},
                {"message", QString("IK: unsupported robot model [%1]").arg(task.robotType)}
            });
            return;
        }
    }

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("Starting inverse kinematics computation for %1 waypoints, backend: %2")
            .arg(task.path.size())
            .arg(KinematicsFactory::backendName())}
    });

    // Build Cartesian pose sequence (position + tool orientation)
    QVector<CartesianPose> poses;
    poses.reserve(task.path.size());
    for (int i = 0; i < task.path.size(); ++i) {
        QVector3D dir = (i + 1 < task.path.size())
            ? (task.path[i+1].position - task.path[i].position)
            : (i > 0 ? (task.path[i].position - task.path[i-1].position) : QVector3D(1,0,0));
        poses.append(KdlKinematics::poseFromNormal(
            task.path[i].position, task.path[i].normal, dir));
    }

    // Use current robot joint angles as the starting configuration (or zero position)
    std::array<double, 6> startJoints;
    const auto& rs = data->robotState();
    for (int i = 0; i < 6; ++i) startJoints[i] = rs.joints[i];

    task.jointTrajectory = m_kinematics->computeTrajectory(poses, startJoints, task.feedRate);

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("IK complete: %1 joint waypoints, total duration %2 s")
            .arg(task.jointTrajectory.size())
            .arg(task.jointTrajectory.isEmpty() ? 0.0
                 : task.jointTrajectory.last().timeFromStart, 0, 'f', 1)}
    });

    emit data->tasksChanged();
}

void PathPlanModule::optimizePath()
{
    auto* data = DataModel::instance();
    if (data->tasks().isEmpty()) return;

    GrindingTask& task = data->currentTask();
    if (task.path.isEmpty()) {
        EventBus::instance()->publish("log.message", {
            {"level", "WARN"},
            {"message", "Please generate a path before optimizing"}
        });
        return;
    }

    QElapsedTimer timer;
    timer.start();

    int beforeCount = task.path.size();
    task.path = m_optimizer.optimize(task.path);
    int afterCount = task.path.size();

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("Path optimization complete: %1 -> %2 points, elapsed %3 ms")
                        .arg(beforeCount)
                        .arg(afterCount)
                        .arg(timer.elapsed())}
    });

    EventBus::instance()->publish("pathplan.path.generated", {});
    emit data->tasksChanged();
}

void PathPlanModule::startSimulation()
{
    auto* data = DataModel::instance();
    if (data->tasks().isEmpty()) return;

    const GrindingTask& task = data->currentTask();
    if (task.path.isEmpty()) {
        EventBus::instance()->publish("log.message", {
            {"level", "WARN"},
            {"message", "No path available to simulate; please generate a path first"}
        });
        return;
    }

    m_simulator->setPath(task.path);
    m_simulator->setSpeedMultiplier(5.0);  // 5x speed simulation
    m_simulator->start();
}

void PathPlanModule::stopSimulation()
{
    m_simulator->stop();
    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "Simulation stopped"}
    });
}
