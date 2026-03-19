#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>

/**
 * @brief Log panel - bottom tab
 */
class LogPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LogPanel(QWidget* parent = nullptr);

    void appendLog(const QString& level, const QString& message);

private:
    QPlainTextEdit* m_logView = nullptr;
    QPushButton* m_clearBtn = nullptr;
};
