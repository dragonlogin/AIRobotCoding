#include "PropertyPanel.h"
#include "core/EventBus.h"
#include "core/DataModel.h"

#include <QVBoxLayout>
#include <QScrollArea>

PropertyPanel::PropertyPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
}

void PropertyPanel::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    // Wrap in QScrollArea to handle overflow content
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
    m_surfacePage = new QGroupBox("Surface Info", this);
    QFormLayout* form = new QFormLayout(m_surfacePage);

    m_surfTypeLabel = new QLabel("-", this);
    m_surfAreaLabel = new QLabel("-", this);
    m_surfCurvLabel = new QLabel("-", this);
    m_surfNormalLabel = new QLabel("-", this);

    form->addRow("Type:", m_surfTypeLabel);
    form->addRow("Area:", m_surfAreaLabel);
    form->addRow("Curvature Range:", m_surfCurvLabel);
    form->addRow("Normal:", m_surfNormalLabel);
}

void PropertyPanel::setupGrindingParams()
{
    m_grindingPage = new QGroupBox("Grinding Parameters", this);
    QFormLayout* form = new QFormLayout(m_grindingPage);

    m_toolTypeCombo = new QComboBox(this);
    m_toolTypeCombo->addItems({
        "Grinding Wheel-80#", "Grinding Wheel-120#", "Grinding Wheel-240#",
        "Abrasive Belt-80#", "Abrasive Belt-120#", "Abrasive Belt-240#",
        "Polishing Wheel-Fine", "Polishing Wheel-Superfine"
    });

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

    form->addRow("Grinding Tool:", m_toolTypeCombo);
    form->addRow("Spindle Speed:", m_spindleSpeedSpin);
    form->addRow("Feed Rate:", m_feedRateSpin);
    form->addRow("Grinding Pressure:", m_pressureSpin);
    form->addRow("Step Over:", m_stepOverSpin);
}

void PropertyPanel::setupRobotStatus()
{
    m_robotPage = new QGroupBox("Robot Status", this);
    QFormLayout* form = new QFormLayout(m_robotPage);

    for (int i = 0; i < 6; ++i) {
        m_jointLabels[i] = new QLabel("0.000°", this);
        form->addRow(QString("J%1:").arg(i + 1), m_jointLabels[i]);
    }

    m_tcpPosLabel = new QLabel("[0.0, 0.0, 0.0]", this);
    m_tcpOriLabel = new QLabel("[0.0, 0.0, 0.0]", this);
    m_robotStatusLabel = new QLabel("Not Connected", this);
    m_robotStatusLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");

    form->addRow("TCP Position:", m_tcpPosLabel);
    form->addRow("TCP Orientation:", m_tcpOriLabel);
    form->addRow("Status:", m_robotStatusLabel);
}

void PropertyPanel::connectSignals()
{
    auto* bus = EventBus::instance();
    auto* data = DataModel::instance();

    // Surface selected -> update surface info
    connect(bus, &EventBus::eventPublished, this,
        [this](const QString& event, const QVariantMap& evData) {
            if (event == "cad.face.selected") {
                int idx = evData.value("faceIndex").toInt();
                const auto& surfaces = DataModel::instance()->surfaces();
                for (const auto& s : surfaces) {
                    if (s.faceIndex == idx) {
                        m_surfTypeLabel->setText(s.surfaceType);
                        m_surfAreaLabel->setText(
                            QString("%1 mm²").arg(s.area, 0, 'f', 2));
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

    // Robot state update
    connect(data, &DataModel::robotStateChanged, this,
        [this](const RobotState& state) {
            for (int i = 0; i < 6; ++i) {
                m_jointLabels[i]->setText(
                    QString("%1°").arg(state.joints[i], 0, 'f', 3));
            }
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
                m_robotStatusLabel->setText("Connected");
                m_robotStatusLabel->setStyleSheet(
                    "color: #2ecc71; font-weight: bold;");
            } else {
                m_robotStatusLabel->setText("Not Connected");
                m_robotStatusLabel->setStyleSheet(
                    "color: #e74c3c; font-weight: bold;");
            }
        });

    // Grinding parameter changed -> update DataModel
    connect(m_toolTypeCombo, &QComboBox::currentTextChanged, this,
        [](const QString& text) {
            auto* data = DataModel::instance();
            if (data->tasks().isEmpty()) return;
            data->currentTask().toolType = text;
        });

    connect(m_spindleSpeedSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [](double val) {
            auto* data = DataModel::instance();
            if (data->tasks().isEmpty()) return;
            data->currentTask().spindleSpeed = val;
        });

    connect(m_feedRateSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [](double val) {
            auto* data = DataModel::instance();
            if (data->tasks().isEmpty()) return;
            data->currentTask().feedRate = val;
        });

    connect(m_pressureSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [](double val) {
            auto* data = DataModel::instance();
            if (data->tasks().isEmpty()) return;
            data->currentTask().pressure = val;
        });

    connect(m_stepOverSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [](double val) {
            auto* data = DataModel::instance();
            if (data->tasks().isEmpty()) return;
            data->currentTask().stepOver = val;
        });
}
