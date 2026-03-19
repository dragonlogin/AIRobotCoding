#pragma once

#include <QWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QFrame>

/**
 * @brief Collapsible section widget - used for grouping in the Model Browser
 */
class CollapsibleSection : public QWidget
{
    Q_OBJECT

public:
    explicit CollapsibleSection(const QString& title, QWidget* parent = nullptr);

    void setContentWidget(QWidget* widget);
    void setExpanded(bool expanded);
    bool isExpanded() const { return m_expanded; }

private:
    QToolButton* m_headerBtn = nullptr;
    QFrame* m_contentFrame = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;
    bool m_expanded = true;
};
