#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QVector3D>
#include <memory>

// 前向声明 OCC 类型
class TopoDS_Shape;

// 运动学数据类型
#include "modules/kinematics/IKinematics.h"

/**
 * @brief 曲面信息
 */
struct SurfaceInfo
{
    int faceIndex = -1;
    QString surfaceType;        // "Plane", "BSpline", "Cylinder" 等
    double area = 0.0;
    double minCurvature = 0.0;
    double maxCurvature = 0.0;
    QVector3D normal;
    bool selectedForGrinding = false;
};

/**
 * @brief 打磨路径点
 */
struct PathPoint
{
    QVector3D position;
    QVector3D normal;
    double feedRate = 500.0;    // mm/min
    double pressure = 10.0;     // N
};

/**
 * @brief 打磨任务
 */
struct GrindingTask
{
    QString name;
    QVector<int> selectedFaces;
    QVector<PathPoint> path;
    QVector<JointWaypoint> jointTrajectory;  // IK 求解后的关节轨迹

    // 打磨参数
    QString toolType = "砂轮-80#";
    double spindleSpeed = 3000;  // rpm
    double feedRate = 500;       // mm/min
    double pressure = 10.0;      // N
    double stepOver = 2.0;       // mm

    // 机器人配置
    QString robotType = "ur5";
};

/**
 * @brief 机器人状态
 */
struct RobotState
{
    double joints[6] = {0};
    QVector3D tcpPosition;
    QVector3D tcpOrientation;   // RPY
    bool connected = false;
    bool moving = false;
    QString statusText = "未连接";
};

/**
 * @brief 全局数据模型 - 集中管理工件、任务、机器人状态
 */
class DataModel : public QObject
{
    Q_OBJECT

public:
    static DataModel* instance();

    // -- 工件模型 --
    void setModelPath(const QString& path);
    QString modelPath() const { return m_modelPath; }

    void setSurfaces(const QVector<SurfaceInfo>& surfaces);
    const QVector<SurfaceInfo>& surfaces() const { return m_surfaces; }

    // -- 打磨任务 --
    void addTask(const GrindingTask& task);
    void removeTask(int index);
    const QVector<GrindingTask>& tasks() const { return m_tasks; }
    GrindingTask& currentTask();
    void setCurrentTaskIndex(int index);

    // -- 机器人状态 --
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
