#pragma once

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/ref_ptr>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>

/**
 * @brief OCC -> OSG geometry converter
 *
 * Converts OpenCASCADE triangulation data into OSG renderable geometry nodes.
 * Each Face produces an independent osg::Geode to support face picking and highlighting.
 */
class OccToOsgConverter
{
public:
    OccToOsgConverter() = default;

    /// Convert an entire Shape to an OSG scene node;
    /// each face becomes an independent child node with faceIndex stored in user data
    osg::ref_ptr<osg::Group> convertShape(const TopoDS_Shape& shape);

    /// Convert a single face to an OSG Geode
    osg::ref_ptr<osg::Geode> convertFace(const TopoDS_Face& face, int faceIndex);

    /// Set the default surface color
    void setDefaultColor(float r, float g, float b, float a = 1.0f);

    /// Set the highlight color
    void setHighlightColor(float r, float g, float b, float a = 1.0f);

private:
    float m_defaultColor[4] = {0.7f, 0.75f, 0.8f, 1.0f};   // silver-gray
    float m_highlightColor[4] = {0.2f, 0.6f, 1.0f, 0.8f};   // blue highlight
};
