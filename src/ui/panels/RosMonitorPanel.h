#pragma once

#include <QWidget>
#include <QTreeWidget>

/**
 * @brief ROS 话题监控面板 - 底部标签页
 */
class RosMonitorPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RosMonitorPanel(QWidget* parent = nullptr);

private:
    QTreeWidget* m_topicTree = nullptr;
};
