#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <QFormLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QGroupBox>

/**
 * @brief 属性面板 - 右侧停靠
 *
 * 根据选中对象动态显示不同属性：
 * - 选中曲面: 显示曲面几何信息
 * - 选中任务: 显示打磨参数编辑器
 * - 机器人状态: 显示关节值和 TCP
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

    // === 曲面信息页 ===
    QWidget* m_surfacePage = nullptr;
    QLabel* m_surfTypeLabel = nullptr;
    QLabel* m_surfAreaLabel = nullptr;
    QLabel* m_surfCurvLabel = nullptr;
    QLabel* m_surfNormalLabel = nullptr;

    // === 打磨参数页 ===
    QWidget* m_grindingPage = nullptr;
    QComboBox* m_toolTypeCombo = nullptr;
    QDoubleSpinBox* m_spindleSpeedSpin = nullptr;
    QDoubleSpinBox* m_feedRateSpin = nullptr;
    QDoubleSpinBox* m_pressureSpin = nullptr;
    QDoubleSpinBox* m_stepOverSpin = nullptr;

    // === 机器人状态页 ===
    QWidget* m_robotPage = nullptr;
    QLabel* m_jointLabels[6] = {};
    QLabel* m_tcpPosLabel = nullptr;
    QLabel* m_tcpOriLabel = nullptr;
    QLabel* m_robotStatusLabel = nullptr;
};
