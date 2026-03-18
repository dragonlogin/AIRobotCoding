#include "DataModel.h"

DataModel* DataModel::s_instance = nullptr;

DataModel::DataModel(QObject* parent)
    : QObject(parent)
{
}

DataModel* DataModel::instance()
{
    if (!s_instance) {
        s_instance = new DataModel();
    }
    return s_instance;
}

void DataModel::setModelPath(const QString& path)
{
    m_modelPath = path;
    emit modelLoaded(path);
}

void DataModel::setSurfaces(const QVector<SurfaceInfo>& surfaces)
{
    m_surfaces = surfaces;
    emit surfacesChanged();
}

void DataModel::addTask(const GrindingTask& task)
{
    m_tasks.append(task);
    emit tasksChanged();
}

void DataModel::removeTask(int index)
{
    if (index >= 0 && index < m_tasks.size()) {
        m_tasks.removeAt(index);
        if (m_currentTaskIndex >= m_tasks.size()) {
            m_currentTaskIndex = m_tasks.size() - 1;
        }
        emit tasksChanged();
    }
}

GrindingTask& DataModel::currentTask()
{
    static GrindingTask dummy;
    if (m_currentTaskIndex >= 0 && m_currentTaskIndex < m_tasks.size()) {
        return m_tasks[m_currentTaskIndex];
    }
    return dummy;
}

void DataModel::setCurrentTaskIndex(int index)
{
    if (index != m_currentTaskIndex) {
        m_currentTaskIndex = index;
        emit currentTaskChanged(index);
    }
}

void DataModel::updateRobotState(const RobotState& state)
{
    m_robotState = state;
    emit robotStateChanged(state);
}
