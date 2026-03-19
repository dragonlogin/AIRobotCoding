#include "LogPanel.h"
#include "core/EventBus.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QEvent>

LogPanel::LogPanel(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);

    QHBoxLayout* toolbar = new QHBoxLayout();
    m_clearBtn = new QPushButton(this);
    m_clearBtn->setFixedWidth(60);
    toolbar->addStretch();
    toolbar->addWidget(m_clearBtn);
    layout->addLayout(toolbar);

    m_logView = new QPlainTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setMaximumBlockCount(5000);
    m_logView->setFont(QFont("Consolas", 9));
    layout->addWidget(m_logView);

    connect(m_clearBtn, &QPushButton::clicked, m_logView, &QPlainTextEdit::clear);

    connect(EventBus::instance(), &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& data) {
            if (event == "log.message")
                appendLog(data.value("level", "INFO").toString(),
                          data.value("message").toString());
        });

    retranslateUi();
}

void LogPanel::retranslateUi()
{
    m_clearBtn->setText(tr("Clear"));
}

void LogPanel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QWidget::changeEvent(event);
}

void LogPanel::appendLog(const QString& level, const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString color = "#cccccc";
    if (level == "WARN")  color = "#f39c12";
    if (level == "ERROR") color = "#e74c3c";

    m_logView->appendHtml(
        QString("<span style='color:#888'>%1</span> "
                "<span style='color:%2'>[%3]</span> %4")
            .arg(timestamp, color, level, message));
}
