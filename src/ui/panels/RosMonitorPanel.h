#pragma once

#include <QWidget>
#include <QTreeWidget>

/**
 * @brief ROS topic monitor panel - bottom tab
 */
class RosMonitorPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RosMonitorPanel(QWidget* parent = nullptr);

private:
    QTreeWidget* m_topicTree = nullptr;
};
