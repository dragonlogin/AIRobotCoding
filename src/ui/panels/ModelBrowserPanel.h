#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>

/**
 * @brief 模型浏览器面板 - 左侧停靠
 *
 * 包含三个折叠区域：
 * 1. 工件树 - 显示 STEP 模型的拓扑结构（实体/面/边）
 * 2. 打磨任务列表
 * 3. 坐标系管理
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

    // 工件模型树
    QTreeWidget* m_modelTree = nullptr;

    // 打磨任务列表
    QTreeWidget* m_taskTree = nullptr;

    // 坐标系树
    QTreeWidget* m_coordTree = nullptr;
};
