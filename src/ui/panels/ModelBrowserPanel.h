#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class CollapsibleSection;

class ModelBrowserPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ModelBrowserPanel(QWidget* parent = nullptr);

protected:
    void changeEvent(QEvent* event) override;

private:
    void setupUI();
    void setupModelTree();
    void setupTaskList();
    void setupCoordinateSection();
    void connectSignals();
    void retranslateUi();

    CollapsibleSection* m_modelSection   = nullptr;
    CollapsibleSection* m_taskSection    = nullptr;
    CollapsibleSection* m_coordSection   = nullptr;
    QTreeWidget*        m_modelTree      = nullptr;
    QTreeWidget*        m_taskTree       = nullptr;
    QTreeWidget*        m_coordTree      = nullptr;
    QPushButton*        m_addTaskBtn     = nullptr;
    QPushButton*        m_removeTaskBtn  = nullptr;
    QTreeWidgetItem*    m_workItem       = nullptr;
    QTreeWidgetItem*    m_toolItem       = nullptr;
};
