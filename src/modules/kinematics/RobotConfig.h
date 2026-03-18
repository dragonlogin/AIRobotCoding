#pragma once

#include <array>
#include <QString>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief 机器人 DH 参数配置
 *
 * 使用标准 DH 参数描述六轴机器人运动学链
 * 参数约定：a(mm), d(mm), alpha(rad), theta_offset(rad)
 */
struct RobotDHConfig
{
    QString name;

    // 每个关节的 DH 参数
    std::array<double, 6> a;            // 连杆长度 mm
    std::array<double, 6> d;            // 连杆偏距 mm
    std::array<double, 6> alpha;        // 连杆扭角 rad
    std::array<double, 6> thetaOffset;  // 关节零位偏移 rad

    // 关节限位 rad
    std::array<double, 6> jointMin;
    std::array<double, 6> jointMax;
};

/**
 * @brief 内置机器人配置库
 */
class RobotConfigLibrary
{
public:
    /// 获取指定型号的 DH 参数（返回 false 表示不支持）
    static bool get(const QString& robotType, RobotDHConfig& config)
    {
        if (robotType == "ur5") {
            config = ur5();
            return true;
        }
        if (robotType == "ur10") {
            config = ur10();
            return true;
        }
        if (robotType == "ur5e") {
            config = ur5e();
            return true;
        }
        return false;
    }

private:
    /// UR5 标准 DH 参数 (单位 mm / rad)
    static RobotDHConfig ur5()
    {
        RobotDHConfig c;
        c.name = "UR5";
        c.a           = {{ 0,       -425.0,  -392.25, 0,      0,      0      }};
        c.d           = {{ 89.159,  0,       0,       109.15, 94.65,  82.3   }};
        c.alpha       = {{ M_PI/2,  0,       0,       M_PI/2, -M_PI/2, 0     }};
        c.thetaOffset = {{ 0,       0,       0,       0,      0,      0      }};
        c.jointMin    = {{ -M_PI,   -M_PI,   -M_PI,   -M_PI,  -M_PI,  -M_PI  }};
        c.jointMax    = {{  M_PI,    M_PI,    M_PI,    M_PI,   M_PI,   M_PI  }};
        return c;
    }

    static RobotDHConfig ur5e()
    {
        RobotDHConfig c;
        c.name = "UR5e";
        c.a           = {{ 0,        -425.0,  -392.25, 0,       0,       0     }};
        c.d           = {{ 162.5,    0,       0,       133.3,   99.7,    99.6  }};
        c.alpha       = {{ M_PI/2,   0,       0,       M_PI/2,  -M_PI/2, 0     }};
        c.thetaOffset = {{ 0,        0,       0,       0,       0,       0     }};
        c.jointMin    = {{ -2*M_PI,  -2*M_PI, -M_PI,   -2*M_PI, -2*M_PI, -2*M_PI }};
        c.jointMax    = {{  2*M_PI,   2*M_PI,  M_PI,    2*M_PI,  2*M_PI,  2*M_PI }};
        return c;
    }

    static RobotDHConfig ur10()
    {
        RobotDHConfig c;
        c.name = "UR10";
        c.a           = {{ 0,       -612.0,  -572.3,  0,       0,      0     }};
        c.d           = {{ 127.3,   0,       0,       163.941, 115.7,  92.2  }};
        c.alpha       = {{ M_PI/2,  0,       0,       M_PI/2,  -M_PI/2, 0    }};
        c.thetaOffset = {{ 0,       0,       0,       0,       0,      0     }};
        c.jointMin    = {{ -M_PI,   -M_PI,   -M_PI,   -M_PI,   -M_PI,  -M_PI }};
        c.jointMax    = {{  M_PI,    M_PI,    M_PI,    M_PI,    M_PI,   M_PI }};
        return c;
    }
};
