#include "RosMonitorPanel.h"
#include "core/EventBus.h"

#include <QVBoxLayout>
#include <QHeaderView>

RosMonitorPanel::RosMonitorPanel(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);

    m_topicTree = new QTreeWidget(this);
    m_topicTree->setHeaderLabels({"Topic Name", "Type", "Hz", "Value"});
    m_topicTree->header()->setStretchLastSection(true);
    m_topicTree->setAlternatingRowColors(true);
    layout->addWidget(m_topicTree);

    // Listen for ROS topic list updates
    connect(EventBus::instance(), &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            if (event == "ros.topics.updated") {
                m_topicTree->clear();
                QVariantList topics = data.value("topics").toList();
                for (const auto& t : topics) {
                    QVariantMap topic = t.toMap();
                    new QTreeWidgetItem(m_topicTree, {
                        topic.value("name").toString(),
                        topic.value("type").toString(),
                        topic.value("hz").toString(),
                        topic.value("value").toString()
                    });
                }
            }
        });
}
