#include "EventBus.h"

EventBus* EventBus::s_instance = nullptr;

EventBus::EventBus(QObject* parent)
    : QObject(parent)
{
}

EventBus* EventBus::instance()
{
    if (!s_instance) {
        s_instance = new EventBus();
    }
    return s_instance;
}

void EventBus::publish(const QString& event, const QVariantMap& data)
{
    emit eventPublished(event, data);
}
