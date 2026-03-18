#include "PathPlanModule.h"
#include "core/EventBus.h"
#include "core/DataModel.h"

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

    // 仿真完成信号
    connect(m_simulator, &PathSimulator::finished, this, []() {
        EventBus::instance()->publish("log.message", {
            {"level", "INFO"},
            {"message", "路径仿真完成"}
        });
    });

    // 监听事件
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

    QAction* genAction = new QAction("生成打磨路径", this);
    connect(genAction, &QAction::triggered, this, &PathPlanModule::generatePath);
    actions.append(genAction);

    QAction* optAction = new QAction("优化路径", this);
    connect(optAction, &QAction::triggered, this, &PathPlanModule::optimizePath);
    actions.append(optAction);

    QAction* simAction = new QAction("仿真运行", this);
    simAction->setShortcut(QKeySequence("F5"));
    connect(simAction, &QAction::triggered, this, &PathPlanModule::startSimulation);
    actions.append(simAction);

    QAction* stopSimAction = new QAction("停止仿真", this);
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
            {"message", "请先创建打磨任务并选择曲面"}
        });
        return;
    }

    GrindingTask& task = data->currentTask();

    bus->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("开始生成打磨路径 [%1]，策略: Zigzag").arg(task.name)}
    });

    QElapsedTimer timer;
    timer.start();

    // 配置路径生成参数
    GrindingPathGenerator::Parameters genParams;
    genParams.strategy = GrindingPathGenerator::Zigzag;
    genParams.stepOver = task.stepOver;
    genParams.feedRate = task.feedRate;
    genParams.pressure = task.pressure;
    genParams.pointDensity = 100;
    genParams.smoothPath = true;
    m_generator.setParameters(genParams);

    // 为选中的每个面生成路径并合并
    QVector<PathPoint> allPaths;

    // 从事件总线请求面数据（由 CadModule 响应）
    // 实际实现中通过 PluginManager 获取 CadModule 引用
    // 这里使用 DataModel 中的面索引
    bus->publish("cad.faces.request", {
        {"faceIndices", QVariant::fromValue(task.selectedFaces)}
    });

    // 生成路径（此处为简化版，实际需获取 TopoDS_Face）
    // 先检查是否有选中的面
    if (task.selectedFaces.isEmpty()) {
        // 如果没有显式选中，对所有面生成
        bus->publish("log.message", {
            {"level", "INFO"},
            {"message", "未选择特定曲面，将对所有曲面生成路径"}
        });
    }

    auto path = m_generator.generate();

    // 路径优化
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
        {"message", QString("路径生成完成: %1 个点, 总长 %2 mm, 预计耗时 %3 s, 耗时 %4 ms")
                        .arg(stats.totalPoints)
                        .arg(stats.totalLength, 0, 'f', 1)
                        .arg(stats.estimatedTime, 0, 'f', 1)
                        .arg(timer.elapsed())}
    });

    // 通知 Viewer 更新路径显示
    bus->publish("pathplan.path.generated", {
        {"taskName", task.name},
        {"pointCount", stats.totalPoints}
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
            {"message", "请先生成路径再进行优化"}
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
        {"message", QString("路径优化完成: %1 -> %2 个点, 耗时 %3 ms")
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
            {"message", "没有可仿真的路径，请先生成路径"}
        });
        return;
    }

    m_simulator->setPath(task.path);
    m_simulator->setSpeedMultiplier(5.0);  // 5倍速仿真
    m_simulator->start();
}

void PathPlanModule::stopSimulation()
{
    m_simulator->stop();
    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "仿真已停止"}
    });
}
