#include "ModelBrowserPanel.h"
#include "core/EventBus.h"
#include "core/DataModel.h"
#include "ui/widgets/CollapsibleSection.h"

#include <QHeaderView>
#include <QPushButton>
#include <QEvent>

ModelBrowserPanel::ModelBrowserPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    retranslateUi();
}

void ModelBrowserPanel::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(0);

    setupModelTree();
    m_modelSection = new CollapsibleSection("", this);
    m_modelSection->setContentWidget(m_modelTree);
    mainLayout->addWidget(m_modelSection);

    setupTaskList();
    m_taskSection = new CollapsibleSection("", this);

    QWidget* taskContainer = new QWidget(this);
    QVBoxLayout* taskLayout = new QVBoxLayout(taskContainer);
    taskLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_addTaskBtn = new QPushButton("+", this);
    m_addTaskBtn->setFixedSize(24, 24);
    m_removeTaskBtn = new QPushButton("-", this);
    m_removeTaskBtn->setFixedSize(24, 24);
    btnLayout->addWidget(m_addTaskBtn);
    btnLayout->addWidget(m_removeTaskBtn);
    btnLayout->addStretch();

    taskLayout->addLayout(btnLayout);
    taskLayout->addWidget(m_taskTree);
    m_taskSection->setContentWidget(taskContainer);
    mainLayout->addWidget(m_taskSection);

    setupCoordinateSection();
    m_coordSection = new CollapsibleSection("", this);
    m_coordSection->setContentWidget(m_coordTree);
    mainLayout->addWidget(m_coordSection);

    mainLayout->addStretch();

    connect(m_addTaskBtn, &QPushButton::clicked, this, []() {
        GrindingTask task;
        task.name = ModelBrowserPanel::tr("Task %1")
                        .arg(DataModel::instance()->tasks().size() + 1);
        DataModel::instance()->addTask(task);
    });
}

void ModelBrowserPanel::setupModelTree()
{
    m_modelTree = new QTreeWidget(this);
    m_modelTree->setHeaderLabels({"", ""});
    m_modelTree->header()->setStretchLastSection(true);
    m_modelTree->setAlternatingRowColors(true);
    m_modelTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_modelTree->setMinimumHeight(200);
    m_modelTree->setContextMenuPolicy(Qt::CustomContextMenu);
}

void ModelBrowserPanel::setupTaskList()
{
    m_taskTree = new QTreeWidget(this);
    m_taskTree->setHeaderLabels({"", "", ""});
    m_taskTree->header()->setStretchLastSection(true);
    m_taskTree->setAlternatingRowColors(true);
    m_taskTree->setMinimumHeight(120);
}

void ModelBrowserPanel::setupCoordinateSection()
{
    m_coordTree = new QTreeWidget(this);
    m_coordTree->setHeaderLabels({"", ""});
    m_coordTree->setMaximumHeight(100);
    m_workItem = new QTreeWidgetItem(m_coordTree, {"", ""});
    m_toolItem  = new QTreeWidgetItem(m_coordTree, {"", ""});
}

void ModelBrowserPanel::retranslateUi()
{
    if (m_modelSection)  m_modelSection->setTitle(tr("Workpiece Model"));
    if (m_taskSection)   m_taskSection->setTitle(tr("Grinding Tasks"));
    if (m_coordSection)  m_coordSection->setTitle(tr("Coordinate Frames"));

    if (m_addTaskBtn)    m_addTaskBtn->setToolTip(tr("New grinding task"));
    if (m_removeTaskBtn) m_removeTaskBtn->setToolTip(tr("Delete selected task"));

    if (m_modelTree) {
        m_modelTree->setHeaderLabels({tr("Name"), tr("Type")});
    }
    if (m_taskTree) {
        m_taskTree->setHeaderLabels({tr("Task Name"), tr("Surface Count"), tr("Status")});
    }
    if (m_coordTree) {
        m_coordTree->setHeaderLabels({tr("Frame"), tr("Type")});
        if (m_workItem) {
            m_workItem->setText(0, tr("Workpiece Frame"));
            m_workItem->setText(1, tr("Fixed"));
        }
        if (m_toolItem) {
            m_toolItem->setText(0, tr("Tool Frame"));
            m_toolItem->setText(1, "TCP");
        }
    }
}

void ModelBrowserPanel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QWidget::changeEvent(event);
}

void ModelBrowserPanel::connectSignals()
{
    auto* data = DataModel::instance();

    connect(data, &DataModel::surfacesChanged, this, [this]() {
        m_modelTree->clear();
        const auto& surfaces = DataModel::instance()->surfaces();
        QTreeWidgetItem* root = new QTreeWidgetItem(m_modelTree,
            {tr("Workpiece"), tr("Solid")});

        for (const auto& surf : surfaces) {
            auto* item = new QTreeWidgetItem(root, {
                tr("Face %1").arg(surf.faceIndex),
                surf.surfaceType
            });
            item->setCheckState(0, surf.selectedForGrinding ? Qt::Checked : Qt::Unchecked);
        }
        m_modelTree->expandAll();
    });

    connect(data, &DataModel::tasksChanged, this, [this]() {
        m_taskTree->clear();
        const auto& tasks = DataModel::instance()->tasks();
        for (int i = 0; i < tasks.size(); ++i) {
            new QTreeWidgetItem(m_taskTree, {
                tasks[i].name,
                QString::number(tasks[i].selectedFaces.size()),
                tasks[i].path.isEmpty() ? tr("Not Planned") : tr("Planned")
            });
        }
    });

    connect(m_modelTree, &QTreeWidget::itemClicked, this,
        [](QTreeWidgetItem* item, int column) {
            Q_UNUSED(column)
            if (item->parent()) {
                int faceIndex = item->text(0).split(" ").last().toInt();
                EventBus::instance()->publish("cad.face.selected",
                    {{"faceIndex", faceIndex}});
            }
        });
}
