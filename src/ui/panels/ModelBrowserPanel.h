#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>

/**
 * @brief Model Browser panel - left dock
 *
 * Contains three collapsible sections:
 * 1. Workpiece tree - shows STEP model topology (solid / face / edge)
 * 2. Grinding task list
 * 3. Coordinate frame management
 */
class ModelBrowserPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ModelBrowserPanel(QWidget* parent = nullptr);

private:
    void setupUI();
    void setupModelTree();
    void setupTaskList();
    void setupCoordinateSection();
    void connectSignals();

    // Workpiece model tree
    QTreeWidget* m_modelTree = nullptr;

    // Grinding task list
    QTreeWidget* m_taskTree = nullptr;

    // Coordinate frame tree
    QTreeWidget* m_coordTree = nullptr;
};
