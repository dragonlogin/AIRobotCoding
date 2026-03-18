#include "ModelBrowserPanel.h"
#include "core/EventBus.h"
#include "core/DataModel.h"
#include "ui/widgets/CollapsibleSection.h"

#include <QHeaderView>
#include <QPushButton>

ModelBrowserPanel::ModelBrowserPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
}

void ModelBrowserPanel::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(0);

    // === 工件树 ===
    setupModelTree();
    CollapsibleSection* modelSection = new CollapsibleSection("工件模型", this);
    modelSection->setContentWidget(m_modelTree);
    mainLayout->addWidget(modelSection);

    // === 打磨任务 ===
    setupTaskList();
    CollapsibleSection* taskSection = new CollapsibleSection("打磨任务", this);

    QWidget* taskContainer = new QWidget(this);
    QVBoxLayout* taskLayout = new QVBoxLayout(taskContainer);
    taskLayout->setContentsMargins(0, 0, 0, 0);

    // 任务操作按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* addBtn = new QPushButton("+", this);
    addBtn->setFixedSize(24, 24);
    addBtn->setToolTip("新建打磨任务");
    QPushButton* removeBtn = new QPushButton("-", this);
    removeBtn->setFixedSize(24, 24);
    removeBtn->setToolTip("删除选中任务");
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(removeBtn);
    btnLayout->addStretch();

    taskLayout->addLayout(btnLayout);
    taskLayout->addWidget(m_taskTree);
    taskSection->setContentWidget(taskContainer);
    mainLayout->addWidget(taskSection);

    // === 坐标系 ===
    setupCoordinateSection();
    CollapsibleSection* coordSection = new CollapsibleSection("坐标系", this);
    coordSection->setContentWidget(m_coordTree);
    mainLayout->addWidget(coordSection);

    mainLayout->addStretch();

    // 新建任务
    connect(addBtn, &QPushButton::clicked, this, []() {
        GrindingTask task;
        task.name = QString("任务 %1").arg(DataModel::instance()->tasks().size() + 1);
        DataModel::instance()->addTask(task);
    });
}

void ModelBrowserPanel::setupModelTree()
{
    m_modelTree = new QTreeWidget(this);
    m_modelTree->setHeaderLabels({"名称", "类型"});
    m_modelTree->header()->setStretchLastSection(true);
    m_modelTree->setAlternatingRowColors(true);
    m_modelTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_modelTree->setMinimumHeight(200);

    // 右键菜单
    m_modelTree->setContextMenuPolicy(Qt::CustomContextMenu);
}

void ModelBrowserPanel::setupTaskList()
{
    m_taskTree = new QTreeWidget(this);
    m_taskTree->setHeaderLabels({"任务名称", "曲面数", "状态"});
    m_taskTree->header()->setStretchLastSection(true);
    m_taskTree->setAlternatingRowColors(true);
    m_taskTree->setMinimumHeight(120);
}

void ModelBrowserPanel::setupCoordinateSection()
{
    m_coordTree = new QTreeWidget(this);
    m_coordTree->setHeaderLabels({"坐标系", "类型"});
    m_coordTree->setMaximumHeight(100);

    auto* workItem = new QTreeWidgetItem(m_coordTree, {"工件坐标系", "固定"});
    auto* toolItem = new QTreeWidgetItem(m_coordTree, {"工具坐标系", "TCP"});
    Q_UNUSED(workItem)
    Q_UNUSED(toolItem)
}

void ModelBrowserPanel::connectSignals()
{
    auto* data = DataModel::instance();

    // 曲面数据变化 -> 更新模型树
    connect(data, &DataModel::surfacesChanged, this, [this]() {
        m_modelTree->clear();
        const auto& surfaces = DataModel::instance()->surfaces();
        QTreeWidgetItem* root = new QTreeWidgetItem(m_modelTree, {"工件", "Solid"});

        for (const auto& surf : surfaces) {
            auto* item = new QTreeWidgetItem(root, {
                QString("面 %1").arg(surf.faceIndex),
                surf.surfaceType
            });
            if (surf.selectedForGrinding) {
                item->setCheckState(0, Qt::Checked);
            } else {
                item->setCheckState(0, Qt::Unchecked);
            }
        }
        m_modelTree->expandAll();
    });

    // 任务变化 -> 更新任务列表
    connect(data, &DataModel::tasksChanged, this, [this]() {
        m_taskTree->clear();
        const auto& tasks = DataModel::instance()->tasks();
        for (int i = 0; i < tasks.size(); ++i) {
            new QTreeWidgetItem(m_taskTree, {
                tasks[i].name,
                QString::number(tasks[i].selectedFaces.size()),
                tasks[i].path.isEmpty() ? "未规划" : "已规划"
            });
        }
    });

    // 模型树选择 -> 发布面选择事件
    connect(m_modelTree, &QTreeWidget::itemClicked, this,
        [](QTreeWidgetItem* item, int column) {
            Q_UNUSED(column)
            if (item->parent()) {  // 子项才是面
                int faceIndex = item->text(0).split(" ").last().toInt();
                EventBus::instance()->publish("cad.face.selected",
                    {{"faceIndex", faceIndex}});
            }
        });
}
