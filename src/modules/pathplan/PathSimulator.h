#pragma once

#include <QObject>
#include <QTimer>
#include <QVector>
#include "core/DataModel.h"

/**
 * @brief Path simulator - simulates path execution
 *
 * Features:
 * - Play back the path at real time or at a speed multiplier
 * - Update the robot pose in real time
 * - Collision detection (simplified)
 * - Simulation data recording
 */
class PathSimulator : public QObject
{
    Q_OBJECT

public:
    explicit PathSimulator(QObject* parent = nullptr);

    /// Set the path to simulate
    void setPath(const QVector<PathPoint>& path);

    /// Set the simulation speed multiplier
    void setSpeedMultiplier(double multiplier) { m_speedMultiplier = multiplier; }

    /// Start simulation
    void start();

    /// Pause simulation
    void pause();

    /// Stop simulation
    void stop();

    /// Seek to the specified progress (0.0 ~ 1.0)
    void seekTo(double progress);

    /// Returns true if the simulation is running
    bool isRunning() const { return m_running; }

    /// Current progress (0.0 ~ 1.0)
    double progress() const;

    /// Current path point index
    int currentIndex() const { return m_currentIndex; }

signals:
    /// Simulation position updated
    void positionUpdated(int pointIndex, const PathPoint& point);

    /// Simulation finished
    void finished();

    /// Simulation progress updated
    void progressChanged(double progress);

private slots:
    void onTimerTick();

private:
    QVector<PathPoint> m_path;
    QTimer* m_timer = nullptr;
    int m_currentIndex = 0;
    double m_speedMultiplier = 1.0;
    bool m_running = false;
    double m_accumulatedTime = 0;
};
