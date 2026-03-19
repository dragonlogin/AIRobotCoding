#pragma once

#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ref_ptr>
#include <QVector>
#include "core/DataModel.h"

/**
 * @brief Path Visualizer - renders grinding paths as 3D lines and arrows
 *
 * Features:
 * - Path line display (color gradient indicating progress)
 * - Normal arrow display
 * - Current execution point marker
 * - Waypoint index labels
 */
class PathVisualizer
{
public:
    PathVisualizer() = default;

    /// Build an OSG scene node from a list of path points
    osg::ref_ptr<osg::Group> createPathNode(const QVector<PathPoint>& path);

    /// Create normal arrow display
    osg::ref_ptr<osg::Geode> createNormalArrows(
        const QVector<PathPoint>& path, float arrowLength = 5.0f);

    /// Create the current execution point marker
    osg::ref_ptr<osg::Geode> createCurrentPointMarker();

    /// Update the position of the current execution point marker
    void updateCurrentPoint(osg::Geode* marker, const PathPoint& point);

    /// Set path line width
    void setLineWidth(float width) { m_lineWidth = width; }

private:
    float m_lineWidth = 2.0f;
};
