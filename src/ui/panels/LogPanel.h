#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>

class LogPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LogPanel(QWidget* parent = nullptr);
    void appendLog(const QString& level, const QString& message);

protected:
    void changeEvent(QEvent* event) override;

private:
    void retranslateUi();

    QPlainTextEdit* m_logView  = nullptr;
    QPushButton*    m_clearBtn = nullptr;
};
