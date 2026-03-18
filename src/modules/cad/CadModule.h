#pragma once

#include "core/PluginInterface.h"
#include "StepReader.h"
#include "SurfaceAnalyzer.h"

#include <QObject>
#include <QMap>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

/**
 * @brief CAD 模块 - 基于 OpenCASCADE 的 STEP 模型管理
 *
 * 职责：
 * - STEP 文件导入/解析
 * - 拓扑遍历（实体/面/边）
 * - 曲面分析（类型、面积、曲率）
 * - OCC Shape -> OSG 网格 转换
 */
class CadModule : public QObject, public IModule
{
    Q_OBJECT
    Q_INTERFACES(IModule)

public:
    explicit CadModule(QObject* parent = nullptr);

    QString moduleId() const override { return "cad"; }
    QString moduleName() const override { return "CAD 模型"; }
    bool initialize() override;
    void shutdown() override;
    QList<QAction*> menuActions() override;

    /// 获取指定索引的 TopoDS_Face
    TopoDS_Face face(int faceIndex) const;

    /// 获取当前加载的形状
    const TopoDS_Shape& currentShape() const { return m_reader.shape(); }

    /// 获取面分析器
    SurfaceAnalyzer* analyzerForFace(int faceIndex);

private:
    void importStepFile(const QString& path);

    StepReader m_reader;
    QVector<TopoDS_Face> m_faces;
    QMap<int, SurfaceAnalyzer*> m_analyzers;
};
