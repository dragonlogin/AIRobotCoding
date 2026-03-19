#pragma once

#include "core/PluginInterface.h"
#include "StepReader.h"
#include "SurfaceAnalyzer.h"

#include <QObject>
#include <QMap>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

/**
 * @brief CAD module - OpenCASCADE-based STEP model management
 *
 * Responsibilities:
 * - STEP file import and parsing
 * - Topology traversal (solids / faces / edges)
 * - Surface analysis (type, area, curvature)
 * - OCC Shape -> OSG mesh conversion
 */
class CadModule : public QObject, public IModule
{
    Q_OBJECT
    Q_INTERFACES(IModule)

public:
    explicit CadModule(QObject* parent = nullptr);

    QString moduleId() const override { return "cad"; }
    QString moduleName() const override { return "CAD Model"; }
    bool initialize() override;
    void shutdown() override;
    QList<QAction*> menuActions() override;

    /// Get the TopoDS_Face at the given index
    TopoDS_Face face(int faceIndex) const;

    /// Get the currently loaded shape
    const TopoDS_Shape& currentShape() const { return m_reader.shape(); }

    /// Get the surface analyzer for the given face index
    SurfaceAnalyzer* analyzerForFace(int faceIndex);

private:
    void importStepFile(const QString& path);

    StepReader m_reader;
    QVector<TopoDS_Face> m_faces;
    QMap<int, SurfaceAnalyzer*> m_analyzers;
};
