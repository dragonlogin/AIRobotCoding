#pragma once

#include <QWidget>
#include <QFormLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QGroupBox>

class PropertyPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyPanel(QWidget* parent = nullptr);

protected:
    void changeEvent(QEvent* event) override;

private:
    void setupUI();
    void setupSurfaceInfo();
    void setupGrindingParams();
    void setupRobotStatus();
    void connectSignals();
    void retranslateUi();

    QGroupBox*   m_surfacePage     = nullptr;
    QFormLayout* m_surfaceForm     = nullptr;
    QLabel*      m_surfTypeLabel   = nullptr;
    QLabel*      m_surfAreaLabel   = nullptr;
    QLabel*      m_surfCurvLabel   = nullptr;
    QLabel*      m_surfNormalLabel = nullptr;

    QGroupBox*      m_grindingPage     = nullptr;
    QFormLayout*    m_grindingForm     = nullptr;
    QComboBox*      m_toolTypeCombo    = nullptr;
    QDoubleSpinBox* m_spindleSpeedSpin = nullptr;
    QDoubleSpinBox* m_feedRateSpin     = nullptr;
    QDoubleSpinBox* m_pressureSpin     = nullptr;
    QDoubleSpinBox* m_stepOverSpin     = nullptr;

    QGroupBox*   m_robotPage        = nullptr;
    QFormLayout* m_robotForm        = nullptr;
    QLabel*      m_jointLabels[6]   = {};
    QLabel*      m_tcpPosLabel      = nullptr;
    QLabel*      m_tcpOriLabel      = nullptr;
    QLabel*      m_robotStatusLabel = nullptr;
};
