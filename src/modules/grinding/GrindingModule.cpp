#include "GrindingModule.h"
#include "core/EventBus.h"
#include "core/DataModel.h"

GrindingModule::GrindingModule(QObject* parent)
    : QObject(parent)
{
}

bool GrindingModule::initialize()
{
    connect(EventBus::instance(), &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            Q_UNUSED(data)
            if (event == "grinding.execute.request") {
                executeGrinding();
            } else if (event == "grinding.stop.request") {
                stopGrinding();
            }
        });

    return true;
}

void GrindingModule::shutdown()
{
}

void GrindingModule::executeGrinding()
{
    auto* data = DataModel::instance();
    if (data->tasks().isEmpty()) {
        EventBus::instance()->publish("log.message", {
            {"level", "WARN"},
            {"message", "没有可执行的打磨任务"}
        });
        return;
    }

    if (!data->robotState().connected) {
        EventBus::instance()->publish("log.message", {
            {"level", "ERROR"},
            {"message", "机器人未连接，无法执行打磨"}
        });
        return;
    }

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "开始执行打磨任务..."}
    });

    // TODO: 将路径发送给机器人执行
    // TODO: 实时监控力控数据
}

void GrindingModule::stopGrinding()
{
    EventBus::instance()->publish("log.message", {
        {"level", "WARN"},
        {"message", "打磨任务已停止"}
    });
}
