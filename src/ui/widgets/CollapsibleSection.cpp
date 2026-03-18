#include "CollapsibleSection.h"

CollapsibleSection::CollapsibleSection(const QString& title, QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 标题按钮
    m_headerBtn = new QToolButton(this);
    m_headerBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_headerBtn->setArrowType(Qt::DownArrow);
    m_headerBtn->setText(title);
    m_headerBtn->setCheckable(true);
    m_headerBtn->setChecked(true);
    m_headerBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_headerBtn->setStyleSheet(
        "QToolButton { background: #3c3f41; border: none; padding: 4px 8px; "
        "color: #ddd; font-weight: bold; text-align: left; }"
        "QToolButton:hover { background: #4a4d4f; }");
    mainLayout->addWidget(m_headerBtn);

    // 内容区域
    m_contentFrame = new QFrame(this);
    m_contentLayout = new QVBoxLayout(m_contentFrame);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_contentFrame);

    connect(m_headerBtn, &QToolButton::toggled, this, [this](bool checked) {
        setExpanded(checked);
    });
}

void CollapsibleSection::setContentWidget(QWidget* widget)
{
    m_contentLayout->addWidget(widget);
}

void CollapsibleSection::setExpanded(bool expanded)
{
    m_expanded = expanded;
    m_contentFrame->setVisible(expanded);
    m_headerBtn->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
}
