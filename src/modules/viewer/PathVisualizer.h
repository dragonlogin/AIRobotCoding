#pragma once

#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ref_ptr>
#include <QVector>
#include "core/DataModel.h"

/**
 * @brief 路径可视化器 - 将打磨路径渲染为3D线条和箭头
 *
 * 功能：
 * - 路径线条显示（彩色渐变表示进度）
 * - 法向箭头显示
 * - 当前执行点标记
 * - 路径点序号标注
 */
class PathVisualizer
{
public:
    PathVisualizer() = default;

    /// 从路径点列表生成 OSG 场景节点
    osg::ref_ptr<osg::Group> createPathNode(const QVector<PathPoint>& path);

    /// 创建法向箭头显示
    osg::ref_ptr<osg::Geode> createNormalArrows(
        const QVector<PathPoint>& path, float arrowLength = 5.0f);

    /// 创建当前执行点标记
    osg::ref_ptr<osg::Geode> createCurrentPointMarker();

    /// 更新当前执行点位置
    void updateCurrentPoint(osg::Geode* marker, const PathPoint& point);

    /// 设置路径线宽
    void setLineWidth(float width) { m_lineWidth = width; }

private:
    float m_lineWidth = 2.0f;
};
