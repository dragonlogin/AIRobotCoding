#pragma once

#include "core/PluginInterface.h"
#include "GrindingPathGenerator.h"
#include "PathOptimizer.h"
#include "PathSimulator.h"
#include "modules/kinematics/IKinematics.h"
#include "modules/kinematics/KinematicsFactory.h"

#include <QObject>
#include <memory>

/**
 * @brief 路径规划模块 - 打磨路径生成、优化与仿真
 *
 * 工作流程：
 * 1. 从 DataModel 获取选中曲面和打磨参数
 * 2. 使用 GrindingPathGenerator 生成初始路径
 * 3. 使用 PathOptimizer 优化路径
 * 4. 更新 DataModel 中的路径数据
 * 5. 通知 Viewer 模块可视化路径
 * 6. 使用 PathSimulator 进行仿真验证
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

    PathSimulator* simulator() const { return m_simulator; }

private:
    void generatePath();
    void optimizePath();
    void startSimulation();
    void stopSimulation();

    void computeIK();

    GrindingPathGenerator m_generator;
    PathOptimizer m_optimizer;
    PathSimulator* m_simulator = nullptr;
    std::unique_ptr<IKinematics> m_kinematics;
};
