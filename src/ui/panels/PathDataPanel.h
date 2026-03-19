#pragma once

#include <QWidget>
#include <QTableWidget>

class PathDataPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PathDataPanel(QWidget* parent = nullptr);

protected:
    void changeEvent(QEvent* event) override;

private:
    void refreshTable();
    void retranslateUi();

    QTableWidget* m_table = nullptr;
};
