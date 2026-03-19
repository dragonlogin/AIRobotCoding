#pragma once

#include <QString>
#include <QVector>
#include "core/DataModel.h"

#include <TopoDS_Shape.hxx>

/**
 * @brief STEP file reader - OpenCASCADE-based
 *
 * Responsibilities:
 * - Read STEP/STP files and return a TopoDS_Shape
 * - Traverse the topology and extract all faces
 * - Analyze the geometric properties of each face (type, area, curvature, normal)
 */
class StepReader
{
public:
    StepReader() = default;

    /// Read a STEP file; returns true on success
    bool load(const QString& filePath);

    /// Get the loaded shape
    const TopoDS_Shape& shape() const { return m_shape; }

    /// Extract geometric information for all faces
    QVector<SurfaceInfo> analyzeSurfaces() const;

    /// Get the error message
    QString errorString() const { return m_errorString; }

private:
    TopoDS_Shape m_shape;
    QString m_errorString;
};
