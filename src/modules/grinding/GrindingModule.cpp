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
            {"message", "No grinding task available to execute"}
        });
        return;
    }

    if (!data->robotState().connected) {
        EventBus::instance()->publish("log.message", {
            {"level", "ERROR"},
            {"message", "Robot not connected; cannot execute grinding"}
        });
        return;
    }

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "Starting grinding task execution..."}
    });

    // TODO: Send path to the robot for execution
    // TODO: Monitor force feedback data in real time
}

void GrindingModule::stopGrinding()
{
    EventBus::instance()->publish("log.message", {
        {"level", "WARN"},
        {"message", "Grinding task stopped"}
    });
}
