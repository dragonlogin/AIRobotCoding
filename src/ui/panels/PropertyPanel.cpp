#include "PropertyPanel.h"
#include "core/EventBus.h"
#include "core/DataModel.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QEvent>

PropertyPanel::PropertyPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    retranslateUi();
}

void PropertyPanel::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    QWidget* content = new QWidget(this);
    QVBoxLayout* contentLayout = new QVBoxLayout(content);

    setupSurfaceInfo();
    setupGrindingParams();
    setupRobotStatus();

    contentLayout->addWidget(m_surfacePage);
    contentLayout->addWidget(m_grindingPage);
    contentLayout->addWidget(m_robotPage);
    contentLayout->addStretch();

    scrollArea->setWidget(content);
    mainLayout->addWidget(scrollArea);
}

void PropertyPanel::setupSurfaceInfo()
{
    m_surfacePage  = new QGroupBox(this);
    m_surfaceForm  = new QFormLayout(m_surfacePage);
    m_surfTypeLabel   = new QLabel("-", this);
    m_surfAreaLabel   = new QLabel("-", this);
    m_surfCurvLabel   = new QLabel("-", this);
    m_surfNormalLabel = new QLabel("-", this);
    m_surfaceForm->addRow(" ", m_surfTypeLabel);
    m_surfaceForm->addRow(" ", m_surfAreaLabel);
    m_surfaceForm->addRow(" ", m_surfCurvLabel);
    m_surfaceForm->addRow(" ", m_surfNormalLabel);
}

void PropertyPanel::setupGrindingParams()
{
    m_grindingPage = new QGroupBox(this);
    m_grindingForm = new QFormLayout(m_grindingPage);

    m_toolTypeCombo = new QComboBox(this);
    // Items are set in retranslateUi

    m_spindleSpeedSpin = new QDoubleSpinBox(this);
    m_spindleSpeedSpin->setRange(100, 12000);
    m_spindleSpeedSpin->setValue(3000);
    m_spindleSpeedSpin->setSuffix(" rpm");
    m_spindleSpeedSpin->setSingleStep(100);

    m_feedRateSpin = new QDoubleSpinBox(this);
    m_feedRateSpin->setRange(10, 5000);
    m_feedRateSpin->setValue(500);
    m_feedRateSpin->setSuffix(" mm/min");
    m_feedRateSpin->setSingleStep(50);

    m_pressureSpin = new QDoubleSpinBox(this);
    m_pressureSpin->setRange(0.5, 100);
    m_pressureSpin->setValue(10);
    m_pressureSpin->setSuffix(" N");
    m_pressureSpin->setSingleStep(0.5);
    m_pressureSpin->setDecimals(1);

    m_stepOverSpin = new QDoubleSpinBox(this);
    m_stepOverSpin->setRange(0.1, 20);
    m_stepOverSpin->setValue(2.0);
    m_stepOverSpin->setSuffix(" mm");
    m_stepOverSpin->setSingleStep(0.1);
    m_stepOverSpin->setDecimals(1);

    m_grindingForm->addRow(" ", m_toolTypeCombo);
    m_grindingForm->addRow(" ", m_spindleSpeedSpin);
    m_grindingForm->addRow(" ", m_feedRateSpin);
    m_grindingForm->addRow(" ", m_pressureSpin);
    m_grindingForm->addRow(" ", m_stepOverSpin);
}

void PropertyPanel::setupRobotStatus()
{
    m_robotPage = new QGroupBox(this);
    m_robotForm = new QFormLayout(m_robotPage);

    for (int i = 0; i < 6; ++i) {
        m_jointLabels[i] = new QLabel("0.000°", this);
        m_robotForm->addRow(" ", m_jointLabels[i]);
    }
    m_tcpPosLabel      = new QLabel("[0.0, 0.0, 0.0]", this);
    m_tcpOriLabel      = new QLabel("[0.0, 0.0, 0.0]", this);
    m_robotStatusLabel = new QLabel(this);
    m_robotStatusLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");

    m_robotForm->addRow(" ", m_tcpPosLabel);
    m_robotForm->addRow(" ", m_tcpOriLabel);
    m_robotForm->addRow(" ", m_robotStatusLabel);
}

static void setFormRowLabel(QFormLayout* form, int row, const QString& text)
{
    auto* item = form->itemAt(row, QFormLayout::LabelRole);
    if (!item) return;
    QLabel* lbl = qobject_cast<QLabel*>(item->widget());
    if (lbl) lbl->setText(text);
}

void PropertyPanel::retranslateUi()
{
    if (m_surfacePage)  m_surfacePage->setTitle(tr("Surface Info"));
    if (m_surfaceForm && m_surfaceForm->rowCount() >= 4) {
        setFormRowLabel(m_surfaceForm, 0, tr("Type:"));
        setFormRowLabel(m_surfaceForm, 1, tr("Area:"));
        setFormRowLabel(m_surfaceForm, 2, tr("Curvature Range:"));
        setFormRowLabel(m_surfaceForm, 3, tr("Normal:"));
    }

    if (m_grindingPage) m_grindingPage->setTitle(tr("Grinding Parameters"));
    if (m_grindingForm && m_grindingForm->rowCount() >= 5) {
        setFormRowLabel(m_grindingForm, 0, tr("Grinding Tool:"));
        setFormRowLabel(m_grindingForm, 1, tr("Spindle Speed:"));
        setFormRowLabel(m_grindingForm, 2, tr("Feed Rate:"));
        setFormRowLabel(m_grindingForm, 3, tr("Grinding Pressure:"));
        setFormRowLabel(m_grindingForm, 4, tr("Step Over:"));
    }
    if (m_toolTypeCombo) {
        int cur = m_toolTypeCombo->currentIndex();
        m_toolTypeCombo->blockSignals(true);
        m_toolTypeCombo->clear();
        m_toolTypeCombo->addItems({
            tr("Grinding Wheel-80#"),  tr("Grinding Wheel-120#"),  tr("Grinding Wheel-240#"),
            tr("Abrasive Belt-80#"),   tr("Abrasive Belt-120#"),   tr("Abrasive Belt-240#"),
            tr("Polishing Wheel-Fine"), tr("Polishing Wheel-Superfine")
        });
        m_toolTypeCombo->setCurrentIndex(cur >= 0 ? cur : 0);
        m_toolTypeCombo->blockSignals(false);
    }

    if (m_robotPage) m_robotPage->setTitle(tr("Robot Status"));
    if (m_robotForm) {
        for (int i = 0; i < 6; ++i)
            setFormRowLabel(m_robotForm, i, tr("J%1:").arg(i + 1));
        setFormRowLabel(m_robotForm, 6, tr("TCP Position:"));
        setFormRowLabel(m_robotForm, 7, tr("TCP Orientation:"));
        setFormRowLabel(m_robotForm, 8, tr("Status:"));
    }
    if (m_robotStatusLabel) {
        bool connected = m_robotStatusLabel->styleSheet().contains("#2ecc71");
        m_robotStatusLabel->setText(connected ? tr("Connected") : tr("Not Connected"));
    }
}

void PropertyPanel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QWidget::changeEvent(event);
}

void PropertyPanel::connectSignals()
{
    auto* bus  = EventBus::instance();
    auto* data = DataModel::instance();

    connect(bus, &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& evData) {
            if (event == "cad.face.selected") {
                int idx = evData.value("faceIndex").toInt();
                for (const auto& s : DataModel::instance()->surfaces()) {
                    if (s.faceIndex == idx) {
                        m_surfTypeLabel->setText(s.surfaceType);
                        m_surfAreaLabel->setText(QString("%1 mm²").arg(s.area, 0, 'f', 2));
                        m_surfCurvLabel->setText(
                            QString("%1 ~ %2").arg(s.minCurvature, 0, 'f', 4)
                                              .arg(s.maxCurvature, 0, 'f', 4));
                        m_surfNormalLabel->setText(
                            QString("[%1, %2, %3]")
                                .arg(s.normal.x(), 0, 'f', 3)
                                .arg(s.normal.y(), 0, 'f', 3)
                                .arg(s.normal.z(), 0, 'f', 3));
                        break;
                    }
                }
            }
        });

    connect(data, &DataModel::robotStateChanged, this,
        [this](const RobotState& state) {
            for (int i = 0; i < 6; ++i)
                m_jointLabels[i]->setText(QString("%1°").arg(state.joints[i], 0, 'f', 3));
            m_tcpPosLabel->setText(
                QString("[%1, %2, %3]")
                    .arg(state.tcpPosition.x(), 0, 'f', 2)
                    .arg(state.tcpPosition.y(), 0, 'f', 2)
                    .arg(state.tcpPosition.z(), 0, 'f', 2));
            m_tcpOriLabel->setText(
                QString("[%1, %2, %3]")
                    .arg(state.tcpOrientation.x(), 0, 'f', 2)
                    .arg(state.tcpOrientation.y(), 0, 'f', 2)
                    .arg(state.tcpOrientation.z(), 0, 'f', 2));
            if (state.connected) {
                m_robotStatusLabel->setText(tr("Connected"));
                m_robotStatusLabel->setStyleSheet("color: #2ecc71; font-weight: bold;");
            } else {
                m_robotStatusLabel->setText(tr("Not Connected"));
                m_robotStatusLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");
            }
        });

    connect(m_toolTypeCombo, &QComboBox::currentTextChanged, this,
        [](const QString& text) {
            auto* d = DataModel::instance();
            if (!d->tasks().isEmpty()) d->currentTask().toolType = text;
        });
    connect(m_spindleSpeedSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [](double v) { auto* d=DataModel::instance(); if(!d->tasks().isEmpty()) d->currentTask().spindleSpeed=v; });
    connect(m_feedRateSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [](double v) { auto* d=DataModel::instance(); if(!d->tasks().isEmpty()) d->currentTask().feedRate=v; });
    connect(m_pressureSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [](double v) { auto* d=DataModel::instance(); if(!d->tasks().isEmpty()) d->currentTask().pressure=v; });
    connect(m_stepOverSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [](double v) { auto* d=DataModel::instance(); if(!d->tasks().isEmpty()) d->currentTask().stepOver=v; });
}
