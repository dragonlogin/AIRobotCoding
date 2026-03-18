#pragma once

#include "core/PluginInterface.h"
#include <QObject>

/**
 * @brief 路径规划模块 - 打磨路径生成与优化
 *
 * 职责：
 * - 基于曲面几何生成打磨路径
 * - 路径优化（平滑、碰撞检测）
 * - 路径仿真播放
 * - 路径导出
 */
class PathPlanModule : public QObject, public IModule
{
    Q_OBJECT
    Q_INTERFACES(IModule)

public:
    explicit PathPlanModule(QObject* parent = nullptr);

    QString moduleId() const override { return "pathplan"; }
    QString moduleName() const override { return "路径规划"; }
    bool initialize() override;
    void shutdown() override;
    QList<QAction*> menuActions() override;

private:
    void generatePath();
    void startSimulation();
    void stopSimulation();
};
