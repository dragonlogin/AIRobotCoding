#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <QFormLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QGroupBox>

/**
 * @brief Property panel - right dock
 *
 * Dynamically displays different properties based on the selected object:
 * - Selected surface: shows surface geometry info
 * - Selected task: shows grinding parameter editor
 * - Robot status: shows joint values and TCP
 */
class PropertyPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyPanel(QWidget* parent = nullptr);

private:
    void setupUI();
    void setupSurfaceInfo();
    void setupGrindingParams();
    void setupRobotStatus();
    void connectSignals();

    QStackedWidget* m_stack = nullptr;

    // === Surface Info page ===
    QWidget* m_surfacePage = nullptr;
    QLabel* m_surfTypeLabel = nullptr;
    QLabel* m_surfAreaLabel = nullptr;
    QLabel* m_surfCurvLabel = nullptr;
    QLabel* m_surfNormalLabel = nullptr;

    // === Grinding Parameters page ===
    QWidget* m_grindingPage = nullptr;
    QComboBox* m_toolTypeCombo = nullptr;
    QDoubleSpinBox* m_spindleSpeedSpin = nullptr;
    QDoubleSpinBox* m_feedRateSpin = nullptr;
    QDoubleSpinBox* m_pressureSpin = nullptr;
    QDoubleSpinBox* m_stepOverSpin = nullptr;

    // === Robot Status page ===
    QWidget* m_robotPage = nullptr;
    QLabel* m_jointLabels[6] = {};
    QLabel* m_tcpPosLabel = nullptr;
    QLabel* m_tcpOriLabel = nullptr;
    QLabel* m_robotStatusLabel = nullptr;
};
