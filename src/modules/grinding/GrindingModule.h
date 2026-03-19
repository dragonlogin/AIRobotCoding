#pragma once

#include "core/PluginInterface.h"
#include <QObject>

/**
 * @brief Grinding execution module - manages actual grinding tasks
 *
 * Responsibilities:
 * - Grinding task execution flow control
 * - Real-time force feedback
 * - Grinding quality monitoring
 */
class GrindingModule : public QObject, public IModule
{
    Q_OBJECT
    Q_INTERFACES(IModule)

public:
    explicit GrindingModule(QObject* parent = nullptr);

    QString moduleId() const override { return "grinding"; }
    QString moduleName() const override { return "Grinding Execution"; }
    bool initialize() override;
    void shutdown() override;

private:
    void executeGrinding();
    void stopGrinding();
};
