#pragma once

#include <QWidget>
#include <QTreeWidget>

class RosMonitorPanel : public QWidget
{
    Q_OBJECT

public:
    explicit RosMonitorPanel(QWidget* parent = nullptr);

protected:
    void changeEvent(QEvent* event) override;

private:
    void retranslateUi();

    QTreeWidget* m_topicTree = nullptr;
};
