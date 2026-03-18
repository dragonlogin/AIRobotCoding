#pragma once

#include <QVector>
#include <QVector3D>
#include "core/DataModel.h"

#include <TopoDS_Face.hxx>

/**
 * @brief 打磨路径生成器
 *
 * 根据曲面几何和打磨参数生成路径点序列。
 *
 * 支持的路径策略：
 * - Raster (栅格扫描)：沿 U 方向等距行扫描
 * - Spiral (螺旋)：从中心向外螺旋
 * - Contour (等高线)：沿等高线扫描
 * - Zigzag (之字形)：交替方向的栅格扫描
 */
class GrindingPathGenerator
{
public:
    /// 路径生成策略
    enum Strategy {
        Raster,     // 栅格扫描
        Zigzag,     // 之字形
        Spiral,     // 螺旋
        Contour     // 等高线
    };

    /// 路径生成参数
    struct Parameters {
        Strategy strategy = Zigzag;
        double stepOver = 2.0;       // 行距 mm
        double feedRate = 500.0;     // 进给速度 mm/min
        double pressure = 10.0;      // 打磨压力 N
        double toolRadius = 25.0;    // 工具半径 mm
        double safeHeight = 20.0;    // 安全高度 mm（抬刀距离）
        double approachAngle = 0.0;  // 进刀角度 deg
        int pointDensity = 100;      // 每行采样点数
        bool smoothPath = true;      // 是否平滑处理
    };

    GrindingPathGenerator() = default;

    /// 设置目标面
    void setFace(const TopoDS_Face& face);

    /// 设置参数
    void setParameters(const Parameters& params) { m_params = params; }

    /// 生成路径
    QVector<PathPoint> generate();

    /// 获取生成统计信息
    struct Statistics {
        int totalPoints = 0;
        double totalLength = 0;     // mm
        double estimatedTime = 0;   // seconds
        double coverageArea = 0;    // mm²
    };
    Statistics statistics() const { return m_stats; }

private:
    QVector<PathPoint> generateRaster();
    QVector<PathPoint> generateZigzag();
    QVector<PathPoint> generateSpiral();
    QVector<PathPoint> generateContour();

    /// 路径平滑（B-spline 插值）
    QVector<PathPoint> smoothPath(const QVector<PathPoint>& path);

    /// 添加进退刀路径
    QVector<PathPoint> addApproachRetract(const QVector<PathPoint>& path);

    /// 计算两点间距离
    static double distance(const PathPoint& a, const PathPoint& b);

    TopoDS_Face m_face;
    Parameters m_params;
    Statistics m_stats;
};
