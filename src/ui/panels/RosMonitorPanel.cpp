#include "RosMonitorPanel.h"
#include "core/EventBus.h"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QEvent>

RosMonitorPanel::RosMonitorPanel(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);

    m_topicTree = new QTreeWidget(this);
    m_topicTree->header()->setStretchLastSection(true);
    m_topicTree->setAlternatingRowColors(true);
    layout->addWidget(m_topicTree);

    connect(EventBus::instance(), &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            if (event == "ros.topics.updated") {
                m_topicTree->clear();
                for (const auto& t : data.value("topics").toList()) {
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

    retranslateUi();
}

void RosMonitorPanel::retranslateUi()
{
    m_topicTree->setHeaderLabels({
        tr("Topic Name"), tr("Type"), tr("Hz"), tr("Value")
    });
}

void RosMonitorPanel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QWidget::changeEvent(event);
}
