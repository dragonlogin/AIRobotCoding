#pragma once

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/ref_ptr>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>

/**
 * @brief OCC -> OSG 几何转换器
 *
 * 将 OpenCASCADE 的三角剖分数据转换为 OSG 可渲染的几何节点。
 * 每个 Face 生成一个独立的 osg::Geode，便于面拾取和高亮。
 */
class OccToOsgConverter
{
public:
    OccToOsgConverter() = default;

    /// 将整个 Shape 转换为 OSG 场景节点
    /// 每个面作为一个独立子节点，用户数据中存储 faceIndex
    osg::ref_ptr<osg::Group> convertShape(const TopoDS_Shape& shape);

    /// 将单个面转换为 OSG Geode
    osg::ref_ptr<osg::Geode> convertFace(const TopoDS_Face& face, int faceIndex);

    /// 设置默认颜色
    void setDefaultColor(float r, float g, float b, float a = 1.0f);

    /// 设置高亮颜色
    void setHighlightColor(float r, float g, float b, float a = 1.0f);

private:
    float m_defaultColor[4] = {0.7f, 0.75f, 0.8f, 1.0f};   // 银灰色
    float m_highlightColor[4] = {0.2f, 0.6f, 1.0f, 0.8f};   // 蓝色高亮
};
