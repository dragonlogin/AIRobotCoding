#include "PathDataPanel.h"
#include "core/DataModel.h"

#include <QVBoxLayout>
#include <QHeaderView>

PathDataPanel::PathDataPanel(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({
        "No.", "X", "Y", "Z", "Nx", "Ny", "Nz", "Feed Rate"
    });
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_table);

    // Refresh when the current task changes
    connect(DataModel::instance(), &DataModel::currentTaskChanged,
            this, [this](int) { refreshTable(); });
}

void PathDataPanel::refreshTable()
{
    m_table->setRowCount(0);
    const auto& task = DataModel::instance()->currentTask();
    const auto& path = task.path;

    m_table->setRowCount(path.size());
    for (int i = 0; i < path.size(); ++i) {
        const auto& pt = path[i];
        m_table->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        m_table->setItem(i, 1, new QTableWidgetItem(QString::number(pt.position.x(), 'f', 3)));
        m_table->setItem(i, 2, new QTableWidgetItem(QString::number(pt.position.y(), 'f', 3)));
        m_table->setItem(i, 3, new QTableWidgetItem(QString::number(pt.position.z(), 'f', 3)));
        m_table->setItem(i, 4, new QTableWidgetItem(QString::number(pt.normal.x(), 'f', 4)));
        m_table->setItem(i, 5, new QTableWidgetItem(QString::number(pt.normal.y(), 'f', 4)));
        m_table->setItem(i, 6, new QTableWidgetItem(QString::number(pt.normal.z(), 'f', 4)));
        m_table->setItem(i, 7, new QTableWidgetItem(QString::number(pt.feedRate, 'f', 1)));
    }
}
