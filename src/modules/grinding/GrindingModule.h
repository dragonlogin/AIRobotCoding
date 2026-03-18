#pragma once

#include "core/PluginInterface.h"
#include <QObject>

/**
 * @brief 打磨执行模块 - 实际打磨任务管理
 *
 * 职责：
 * - 打磨任务执行流程控制
 * - 实时力控反馈
 * - 打磨质量监控
 */
class GrindingModule : public QObject, public IModule
{
    Q_OBJECT
    Q_INTERFACES(IModule)

public:
    explicit GrindingModule(QObject* parent = nullptr);

    QString moduleId() const override { return "grinding"; }
    QString moduleName() const override { return "打磨执行"; }
    bool initialize() override;
    void shutdown() override;

private:
    void executeGrinding();
    void stopGrinding();
};
