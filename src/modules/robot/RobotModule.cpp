#include "RobotModule.h"
#include "RosBridge.h"
#include "core/EventBus.h"
#include "core/DataModel.h"

#include <QAction>
#include <QtMath>

RobotModule::RobotModule(QObject* parent)
    : QObject(parent)
{
}

bool RobotModule::initialize()
{
    m_bridge = new RosBridge(this);

    // RosBridge signals -> DataModel updates
    connect(m_bridge, &RosBridge::jointStateUpdated, this,
        [](const RobotState& state) {
            DataModel::instance()->updateRobotState(state);
        });

    connect(m_bridge, &RosBridge::connectionChanged, this,
        [](bool connected) {
            RobotState state;
            state.connected = connected;
            state.statusText = connected ? "Connected" : "Disconnected";
            DataModel::instance()->updateRobotState(state);
        });

    connect(m_bridge, &RosBridge::errorOccurred, this,
        [](const QString& error) {
            EventBus::instance()->publish("log.message", {
                {"level", "ERROR"}, {"message", error}
            });
        });

    // Force sensor data -> EventBus
    connect(m_bridge, &RosBridge::wrenchUpdated, this,
        [](double fx, double fy, double fz, double tx, double ty, double tz) {
            EventBus::instance()->publish("robot.wrench.updated", {
                {"fx", fx}, {"fy", fy}, {"fz", fz},
                {"tx", tx}, {"ty", ty}, {"tz", tz}
            });
        });

    // Listen to the event bus
    connect(EventBus::instance(), &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            if (event == "robot.connect.request") {
                connectToRos(data.value("masterUri", "localhost:11311").toString());
            } else if (event == "robot.disconnect.request") {
                disconnectRos();
            } else if (event == "robot.estop") {
                emergencyStop();
            } else if (event == "robot.send.trajectory") {
                // Extract trajectory from event data and send
                // In practice, call the bridge interface directly
            }
        });

    // Simulation mode timer
    m_simTimer = new QTimer(this);
    connect(m_simTimer, &QTimer::timeout, this, [this]() {
        m_simTime += 0.05;  // 50ms

        RobotState state;
        state.connected = true;
        state.statusText = "Simulation Running";

        // Simulate joint motion (sine wave)
        for (int i = 0; i < 6; ++i) {
            state.joints[i] = 30.0 * qSin(m_simTime * (0.5 + i * 0.1));
        }

        // Simulated TCP position
        state.tcpPosition = QVector3D(
            static_cast<float>(400 + 100 * qCos(m_simTime * 0.3)),
            static_cast<float>(200 * qSin(m_simTime * 0.3)),
            static_cast<float>(300 + 50 * qSin(m_simTime * 0.5)));
        state.tcpOrientation = QVector3D(0, 0,
            static_cast<float>(m_simTime * 10));

        DataModel::instance()->updateRobotState(state);
    });

    return true;
}

void RobotModule::shutdown()
{
    m_simTimer->stop();
    if (m_bridge) {
        m_bridge->disconnect();
    }
}

QList<QAction*> RobotModule::toolBarActions()
{
    QList<QAction*> actions;

    QAction* connectAction = new QAction("Connect", this);
    connect(connectAction, &QAction::triggered, this, [this]() {
        if (m_bridge->isConnected()) {
            disconnectRos();
        } else {
            connectToRos("localhost:11311");
        }
    });
    actions.append(connectAction);

    QAction* eStopAction = new QAction("Emergency Stop", this);
    connect(eStopAction, &QAction::triggered, this, &RobotModule::emergencyStop);
    actions.append(eStopAction);

    return actions;
}

QList<QAction*> RobotModule::menuActions()
{
    QList<QAction*> actions;

    QAction* simAction = new QAction("Start Simulation Mode", this);
    connect(simAction, &QAction::triggered, this, &RobotModule::startSimulation);
    actions.append(simAction);

    return actions;
}

void RobotModule::connectToRos(const QString& masterUri)
{
    m_bridge->connectToMaster(masterUri);
}

void RobotModule::disconnectRos()
{
    m_simTimer->stop();
    m_bridge->disconnect();
}

void RobotModule::emergencyStop()
{
    m_simTimer->stop();
    m_bridge->emergencyStop();

    EventBus::instance()->publish("log.message", {
        {"level", "WARN"},
        {"message", "Emergency stop triggered! All motion halted."}
    });
}

void RobotModule::startSimulation()
{
    m_simTime = 0;

    RobotState state;
    state.connected = true;
    state.statusText = "Simulation Mode";
    DataModel::instance()->updateRobotState(state);

    m_simTimer->start(50);  // 20Hz

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", "Robot simulation mode started (20Hz)"}
    });
}
