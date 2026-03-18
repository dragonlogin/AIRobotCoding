#pragma once

#include <QString>
#include <QVector>
#include "core/DataModel.h"

#include <TopoDS_Shape.hxx>

/**
 * @brief STEP 文件读取器 - 基于 OpenCASCADE
 *
 * 职责：
 * - 读取 STEP/STP 文件并返回 TopoDS_Shape
 * - 遍历拓扑结构，提取所有面
 * - 分析每个面的几何属性（类型、面积、曲率、法向）
 */
class StepReader
{
public:
    StepReader() = default;

    /// 读取 STEP 文件，返回是否成功
    bool load(const QString& filePath);

    /// 获取读取到的形状
    const TopoDS_Shape& shape() const { return m_shape; }

    /// 提取所有面的几何信息
    QVector<SurfaceInfo> analyzeSurfaces() const;

    /// 获取错误信息
    QString errorString() const { return m_errorString; }

private:
    TopoDS_Shape m_shape;
    QString m_errorString;
};
