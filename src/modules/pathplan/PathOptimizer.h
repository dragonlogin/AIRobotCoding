#pragma once

#include <QVector>
#include "core/DataModel.h"

/**
 * @brief 路径优化器
 *
 * 对生成的打磨路径进行后处理优化：
 * - 去除冗余点
 * - 速度规划（根据曲率自适应调速）
 * - 加减速过渡
 * - 压力自适应（根据曲率调整打磨压力）
 */
class PathOptimizer
{
public:
    /// 优化参数
    struct Parameters {
        double minPointDistance = 0.5;      // 最小点间距 mm
        double maxPointDistance = 10.0;     // 最大点间距 mm
        double maxFeedRate = 1000.0;       // 最大进给速度 mm/min
        double minFeedRate = 50.0;         // 最小进给速度 mm/min
        double acceleration = 500.0;       // 加速度 mm/s²
        double curvatureSpeedFactor = 0.5; // 曲率对速度的影响系数
        double curvaturePressureFactor = 1.0; // 曲率对压力的影响系数
        double basePressure = 10.0;        // 基础打磨压力 N
    };

    PathOptimizer() = default;

    void setParameters(const Parameters& params) { m_params = params; }

    /// 执行全部优化
    QVector<PathPoint> optimize(const QVector<PathPoint>& path);

    /// 去除冗余点（距离过近的点）
    QVector<PathPoint> removeRedundantPoints(const QVector<PathPoint>& path);

    /// 曲率自适应调速
    QVector<PathPoint> adaptiveFeedRate(const QVector<PathPoint>& path);

    /// 梯形加减速规划
    QVector<PathPoint> trapezoidalVelocity(const QVector<PathPoint>& path);

    /// 曲率自适应压力
    QVector<PathPoint> adaptivePressure(const QVector<PathPoint>& path);

private:
    /// 估算路径点处的局部曲率
    double estimateCurvature(const QVector<PathPoint>& path, int index);

    Parameters m_params;
};
