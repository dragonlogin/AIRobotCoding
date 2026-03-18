#pragma once

#include <QWidget>
#include <QTableWidget>

/**
 * @brief 路径数据面板 - 底部标签页
 *
 * 显示打磨路径点的详细数据表格
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
