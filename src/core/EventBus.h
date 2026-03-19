#pragma once

#include <QObject>
#include <QVariantMap>

/**
 * @brief Global event bus - decoupled inter-module communication
 *
 * Modules publish/subscribe to events via the event bus, avoiding direct dependencies.
 * Event types use string namespace convention, e.g. "cad.model.loaded", "robot.state.changed"
 */
class EventBus : public QObject
{
    Q_OBJECT

public:
    static EventBus* instance();

    /// Publish an event
    void publish(const QString& event, const QVariantMap& data = {});

signals:
    /// Unified event signal; subscribers filter by event name
    void eventPublished(const QString& event, const QVariantMap& data);

private:
    EventBus(QObject* parent = nullptr);
    static EventBus* s_instance;
};
