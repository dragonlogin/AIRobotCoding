#pragma once

#include <QVector>
#include <QVector3D>
#include <TopoDS_Face.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

/**
 * @brief 曲面分析器 - 提供打磨相关的曲面几何分析功能
 *
 * 职责：
 * - UV 参数域采样
 * - 法向量场计算
 * - 曲率场计算
 * - 曲面离散化（为路径规划提供数据）
 */
class SurfaceAnalyzer
{
public:
    struct SamplePoint {
        gp_Pnt position;
        gp_Vec normal;
        double minCurvature = 0;
        double maxCurvature = 0;
        double u = 0;
        double v = 0;
    };

    SurfaceAnalyzer() = default;

    /// 设置要分析的面
    void setFace(const TopoDS_Face& face);

    /// 获取 UV 参数域范围
    void uvBounds(double& uMin, double& uMax, double& vMin, double& vMax) const;

    /// 在指定 UV 坐标处求值
    SamplePoint evaluate(double u, double v) const;

    /// 均匀采样曲面，返回采样点网格
    QVector<QVector<SamplePoint>> sampleGrid(int uCount, int vCount) const;

    /// 沿 U 方向生成等距线（用于生成打磨行路径）
    QVector<QVector<SamplePoint>> generateIsoLines(
        int lineCount, int pointsPerLine) const;

    /// 计算指定点处的工具接触法向（考虑工具半径补偿）
    gp_Vec toolNormal(double u, double v, double toolRadius) const;

private:
    TopoDS_Face m_face;
};
