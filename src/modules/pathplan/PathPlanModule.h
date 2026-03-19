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
 * @brief Path planning module - grinding path generation, optimization, and simulation
 *
 * Workflow:
 * 1. Retrieve selected surfaces and grinding parameters from DataModel
 * 2. Generate the initial path using GrindingPathGenerator
 * 3. Optimize the path using PathOptimizer
 * 4. Update path data in DataModel
 * 5. Notify the Viewer module to visualize the path
 * 6. Validate the path using PathSimulator
 */
class PathPlanModule : public QObject, public IModule
{
    Q_OBJECT
    Q_INTERFACES(IModule)

public:
    explicit PathPlanModule(QObject* parent = nullptr);

    QString moduleId() const override { return "pathplan"; }
    QString moduleName() const override { return "Path Planning"; }
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
