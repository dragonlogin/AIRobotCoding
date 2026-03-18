#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>

/**
 * @brief 日志面板 - 底部标签页
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
