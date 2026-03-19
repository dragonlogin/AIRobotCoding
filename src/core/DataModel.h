#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QVector3D>
#include <memory>

// Forward declaration of OCC types
class TopoDS_Shape;

// Kinematics data types
#include "modules/kinematics/IKinematics.h"

/**
 * @brief Surface information
 */
struct SurfaceInfo
{
    int faceIndex = -1;
    QString surfaceType;        // "Plane", "BSpline", "Cylinder", etc.
    double area = 0.0;
    double minCurvature = 0.0;
    double maxCurvature = 0.0;
    QVector3D normal;
    bool selectedForGrinding = false;
};

/**
 * @brief Grinding path point
 */
struct PathPoint
{
    QVector3D position;
    QVector3D normal;
    double feedRate = 500.0;    // mm/min
    double pressure = 10.0;     // N
};

/**
 * @brief Grinding task
 */
struct GrindingTask
{
    QString name;
    QVector<int> selectedFaces;
    QVector<PathPoint> path;
    QVector<JointWaypoint> jointTrajectory;  // Joint trajectory after IK solve

    // Grinding parameters
    QString toolType = "GrindingWheel-80#";
    double spindleSpeed = 3000;  // rpm
    double feedRate = 500;       // mm/min
    double pressure = 10.0;      // N
    double stepOver = 2.0;       // mm

    // Robot configuration
    QString robotType = "ur5";
};

/**
 * @brief Robot state
 */
struct RobotState
{
    double joints[6] = {0};
    QVector3D tcpPosition;
    QVector3D tcpOrientation;   // RPY
    bool connected = false;
    bool moving = false;
    QString statusText = "Not Connected";
};

/**
 * @brief Global data model - centrally manages workpiece, tasks, and robot state
 */
class DataModel : public QObject
{
    Q_OBJECT

public:
    static DataModel* instance();

    // -- Workpiece model --
    void setModelPath(const QString& path);
    QString modelPath() const { return m_modelPath; }

    void setSurfaces(const QVector<SurfaceInfo>& surfaces);
    const QVector<SurfaceInfo>& surfaces() const { return m_surfaces; }

    // -- Grinding tasks --
    void addTask(const GrindingTask& task);
    void removeTask(int index);
    const QVector<GrindingTask>& tasks() const { return m_tasks; }
    GrindingTask& currentTask();
    void setCurrentTaskIndex(int index);

    // -- Robot state --
    void updateRobotState(const RobotState& state);
    const RobotState& robotState() const { return m_robotState; }

signals:
    void modelLoaded(const QString& path);
    void surfacesChanged();
    void tasksChanged();
    void currentTaskChanged(int index);
    void robotStateChanged(const RobotState& state);

private:
    DataModel(QObject* parent = nullptr);
    static DataModel* s_instance;

    QString m_modelPath;
    QVector<SurfaceInfo> m_surfaces;
    QVector<GrindingTask> m_tasks;
    int m_currentTaskIndex = -1;
    RobotState m_robotState;
};
