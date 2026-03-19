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

    // === Workpiece tree ===
    setupModelTree();
    CollapsibleSection* modelSection = new CollapsibleSection("Workpiece Model", this);
    modelSection->setContentWidget(m_modelTree);
    mainLayout->addWidget(modelSection);

    // === Grinding tasks ===
    setupTaskList();
    CollapsibleSection* taskSection = new CollapsibleSection("Grinding Tasks", this);

    QWidget* taskContainer = new QWidget(this);
    QVBoxLayout* taskLayout = new QVBoxLayout(taskContainer);
    taskLayout->setContentsMargins(0, 0, 0, 0);

    // Task action buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* addBtn = new QPushButton("+", this);
    addBtn->setFixedSize(24, 24);
    addBtn->setToolTip("New grinding task");
    QPushButton* removeBtn = new QPushButton("-", this);
    removeBtn->setFixedSize(24, 24);
    removeBtn->setToolTip("Delete selected task");
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(removeBtn);
    btnLayout->addStretch();

    taskLayout->addLayout(btnLayout);
    taskLayout->addWidget(m_taskTree);
    taskSection->setContentWidget(taskContainer);
    mainLayout->addWidget(taskSection);

    // === Coordinate frames ===
    setupCoordinateSection();
    CollapsibleSection* coordSection = new CollapsibleSection("Coordinate Frames", this);
    coordSection->setContentWidget(m_coordTree);
    mainLayout->addWidget(coordSection);

    mainLayout->addStretch();

    // Create new task
    connect(addBtn, &QPushButton::clicked, this, []() {
        GrindingTask task;
        task.name = QString("Task %1").arg(DataModel::instance()->tasks().size() + 1);
        DataModel::instance()->addTask(task);
    });
}

void ModelBrowserPanel::setupModelTree()
{
    m_modelTree = new QTreeWidget(this);
    m_modelTree->setHeaderLabels({"Name", "Type"});
    m_modelTree->header()->setStretchLastSection(true);
    m_modelTree->setAlternatingRowColors(true);
    m_modelTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_modelTree->setMinimumHeight(200);

    // Context menu
    m_modelTree->setContextMenuPolicy(Qt::CustomContextMenu);
}

void ModelBrowserPanel::setupTaskList()
{
    m_taskTree = new QTreeWidget(this);
    m_taskTree->setHeaderLabels({"Task Name", "Surface Count", "Status"});
    m_taskTree->header()->setStretchLastSection(true);
    m_taskTree->setAlternatingRowColors(true);
    m_taskTree->setMinimumHeight(120);
}

void ModelBrowserPanel::setupCoordinateSection()
{
    m_coordTree = new QTreeWidget(this);
    m_coordTree->setHeaderLabels({"Frame", "Type"});
    m_coordTree->setMaximumHeight(100);

    auto* workItem = new QTreeWidgetItem(m_coordTree, {"Workpiece Frame", "Fixed"});
    auto* toolItem = new QTreeWidgetItem(m_coordTree, {"Tool Frame", "TCP"});
    Q_UNUSED(workItem)
    Q_UNUSED(toolItem)
}

void ModelBrowserPanel::connectSignals()
{
    auto* data = DataModel::instance();

    // Surface data changed -> update model tree
    connect(data, &DataModel::surfacesChanged, this, [this]() {
        m_modelTree->clear();
        const auto& surfaces = DataModel::instance()->surfaces();
        QTreeWidgetItem* root = new QTreeWidgetItem(m_modelTree, {"Workpiece", "Solid"});

        for (const auto& surf : surfaces) {
            auto* item = new QTreeWidgetItem(root, {
                QString("Face %1").arg(surf.faceIndex),
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

    // Tasks changed -> update task list
    connect(data, &DataModel::tasksChanged, this, [this]() {
        m_taskTree->clear();
        const auto& tasks = DataModel::instance()->tasks();
        for (int i = 0; i < tasks.size(); ++i) {
            new QTreeWidgetItem(m_taskTree, {
                tasks[i].name,
                QString::number(tasks[i].selectedFaces.size()),
                tasks[i].path.isEmpty() ? "Not Planned" : "Planned"
            });
        }
    });

    // Model tree selection -> publish face selected event
    connect(m_modelTree, &QTreeWidget::itemClicked, this,
        [](QTreeWidgetItem* item, int column) {
            Q_UNUSED(column)
            if (item->parent()) {  // Only child items are faces
                int faceIndex = item->text(0).split(" ").last().toInt();
                EventBus::instance()->publish("cad.face.selected",
                    {{"faceIndex", faceIndex}});
            }
        });
}
