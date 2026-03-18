#pragma once

#include <QObject>
#include <QVariantMap>

/**
 * @brief 全局事件总线 - 模块间解耦通信
 *
 * 各模块通过事件总线发布/订阅事件，避免直接依赖。
 * 事件类型采用字符串命名空间约定，如 "cad.model.loaded", "robot.state.changed"
 */
class EventBus : public QObject
{
    Q_OBJECT

public:
    static EventBus* instance();

    /// 发布事件
    void publish(const QString& event, const QVariantMap& data = {});

signals:
    /// 统一事件信号，订阅者通过 event 名称过滤
    void eventPublished(const QString& event, const QVariantMap& data);

private:
    EventBus(QObject* parent = nullptr);
    static EventBus* s_instance;
};
