#include "PathPlanModule.h"
#include "core/EventBus.h"
#include "core/DataModel.h"

#include <QAction>

PathPlanModule::PathPlanModule(QObject* parent)
    : QObject(parent)
{
}

bool PathPlanModule::initialize()
{
    connect(EventBus::instance(), &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            Q_UNUSED(data)
            if (event == "pathplan.generate.request") {
                generatePath();
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
}

QList<QAction*> PathPlanModule::menuActions()
{
    QList<QAction*> actions;

    QAction* genAction = new QAction("生成打磨路径", this);
    connect(genAction, &QAction::triggered, this, &PathPlanModule::generatePath);
    actions.append(genAction);

    QAction* simAction = new QAction("仿真运行", this);
    simAction->setShortcut(QKeySequence("F5"));
    connect(simAction, &QAction::triggered, this, &PathPlanModule::startSimulation);
    actions.append(simAction);

    return actions;
}

void PathPlanModule::generatePath()
{
    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "开始生成打磨路径..."}
    });

    // TODO: 根据选中曲面和打磨参数生成路径
    // 1. 获取曲面 UV 参数化
    // 2. 按行距生成等距路径线
    // 3. 沿法线方向偏置工具
    // 4. 逆运动学求解关节角

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "路径生成完成"}
    });
}

void PathPlanModule::startSimulation()
{
    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "开始路径仿真..."}
    });

    // TODO: 定时器驱动路径点回放，更新机器人姿态
}

void PathPlanModule::stopSimulation()
{
    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "仿真已停止"}
    });
}
