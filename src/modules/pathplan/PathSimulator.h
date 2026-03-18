#pragma once

#include <QObject>
#include <QTimer>
#include <QVector>
#include "core/DataModel.h"

/**
 * @brief 路径仿真器 - 模拟路径执行过程
 *
 * 功能：
 * - 按实际时间或倍速播放路径
 * - 实时更新机器人位姿
 * - 碰撞检测（简化版）
 * - 仿真数据记录
 */
class PathSimulator : public QObject
{
    Q_OBJECT

public:
    explicit PathSimulator(QObject* parent = nullptr);

    /// 设置要仿真的路径
    void setPath(const QVector<PathPoint>& path);

    /// 设置仿真速度倍率
    void setSpeedMultiplier(double multiplier) { m_speedMultiplier = multiplier; }

    /// 开始仿真
    void start();

    /// 暂停仿真
    void pause();

    /// 停止仿真
    void stop();

    /// 跳转到指定进度 (0.0 ~ 1.0)
    void seekTo(double progress);

    /// 是否正在运行
    bool isRunning() const { return m_running; }

    /// 当前进度 (0.0 ~ 1.0)
    double progress() const;

    /// 当前路径点索引
    int currentIndex() const { return m_currentIndex; }

signals:
    /// 仿真位置更新
    void positionUpdated(int pointIndex, const PathPoint& point);

    /// 仿真完成
    void finished();

    /// 仿真进度更新
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
