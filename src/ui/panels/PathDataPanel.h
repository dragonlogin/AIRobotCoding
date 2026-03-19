#pragma once

#include <QWidget>
#include <QTableWidget>

/**
 * @brief Path Data panel - bottom tab
 *
 * Displays a detailed data table of grinding path waypoints
 */
class PathDataPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PathDataPanel(QWidget* parent = nullptr);

private:
    QTableWidget* m_table = nullptr;
    void refreshTable();
};
