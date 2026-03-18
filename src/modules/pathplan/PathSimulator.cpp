#include "PathSimulator.h"
#include "core/EventBus.h"
#include "core/DataModel.h"

PathSimulator::PathSimulator(QObject* parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(20);  // 50Hz 更新
    connect(m_timer, &QTimer::timeout, this, &PathSimulator::onTimerTick);
}

void PathSimulator::setPath(const QVector<PathPoint>& path)
{
    m_path = path;
    m_currentIndex = 0;
    m_accumulatedTime = 0;
}

void PathSimulator::start()
{
    if (m_path.isEmpty()) return;

    m_running = true;
    m_timer->start();

    EventBus::instance()->publish("log.message", {
        {"level", "INFO"},
        {"message", QString("仿真开始，共 %1 个路径点，速度倍率: %2x")
                        .arg(m_path.size())
                        .arg(m_speedMultiplier)}
    });
}

void PathSimulator::pause()
{
    m_running = false;
    m_timer->stop();
}

void PathSimulator::stop()
{
    m_running = false;
    m_timer->stop();
    m_currentIndex = 0;
    m_accumulatedTime = 0;
}

void PathSimulator::seekTo(double prog)
{
    int index = static_cast<int>(prog * (m_path.size() - 1));
    m_currentIndex = qBound(0, index, m_path.size() - 1);

    if (!m_path.isEmpty()) {
        emit positionUpdated(m_currentIndex, m_path[m_currentIndex]);
        emit progressChanged(progress());
    }
}

double PathSimulator::progress() const
{
    if (m_path.isEmpty()) return 0;
    return static_cast<double>(m_currentIndex) / (m_path.size() - 1);
}

void PathSimulator::onTimerTick()
{
    if (!m_running || m_path.isEmpty()) return;

    if (m_currentIndex >= m_path.size()) {
        stop();
        emit finished();
        EventBus::instance()->publish("log.message", {
            {"level", "INFO"},
            {"message", "仿真完成"}
        });
        return;
    }

    const PathPoint& point = m_path[m_currentIndex];

    // 更新机器人 TCP 位姿（仿真）
    RobotState state = DataModel::instance()->robotState();
    state.tcpPosition = point.position;
    state.tcpOrientation = point.normal * 90.0f;  // 简化姿态表示
    state.moving = true;
    state.statusText = QString("仿真 %1/%2").arg(m_currentIndex + 1).arg(m_path.size());
    DataModel::instance()->updateRobotState(state);

    emit positionUpdated(m_currentIndex, point);
    emit progressChanged(progress());

    // 计算下一个点的时间间隔
    if (m_currentIndex < m_path.size() - 1) {
        QVector3D diff = m_path[m_currentIndex + 1].position - point.position;
        double dist = static_cast<double>(diff.length());
        double feedRate = point.feedRate / 60.0;  // mm/min -> mm/s

        if (feedRate > 0.001) {
            double dt = dist / (feedRate * m_speedMultiplier);
            m_accumulatedTime += 0.02;  // 20ms

            if (m_accumulatedTime >= dt) {
                m_accumulatedTime -= dt;
                ++m_currentIndex;
            }
        } else {
            ++m_currentIndex;
        }
    } else {
        ++m_currentIndex;
    }
}
