add_library(mod_kinematics STATIC
    IKinematics.h
    RobotConfig.h
    KdlKinematics.h
    KdlKinematics.cpp
    KinematicsFactory.h
    KinematicsFactory.cpp
)

target_include_directories(mod_kinematics PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

# ============================================================================
# KDL 支持（macOS: brew install orocos-kdl，Ubuntu: apt install liborocos-kdl-dev）
# ============================================================================
find_package(orocos_kdl QUIET)

target_link_libraries(mod_kinematics Qt5::Core Qt5::Gui)
target_include_directories(mod_kinematics PUBLIC
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
)

if(orocos_kdl_FOUND)
    message(STATUS "KDL found: ${orocos_kdl_VERSION} — 使用 KDL 运动学解算器")
    find_package(Eigen3 REQUIRED)
    target_compile_definitions(mod_kinematics PUBLIC HAS_KDL)
    target_include_directories(mod_kinematics PRIVATE
        ${orocos_kdl_INCLUDE_DIRS}
        ${EIGEN3_INCLUDE_DIR}
    )
    target_link_libraries(mod_kinematics ${orocos_kdl_LIBRARIES} Eigen3::Eigen)
else()
    message(STATUS "KDL 未找到 — 使用内置数值 IK（降级模式）")
    message(STATUS "  macOS 安装: brew install orocos-kdl")
    message(STATUS "  Ubuntu 安装: sudo apt install liborocos-kdl-dev")
endif()

# ============================================================================
# 未来 ROS/MoveIt 扩展入口（仅 Ubuntu）
# ============================================================================
if(catkin_FOUND)
    # TODO: 添加 MoveItKinematics 实现
    # target_compile_definitions(mod_kinematics PUBLIC HAS_ROS)
    # target_link_libraries(mod_kinematics ${catkin_LIBRARIES})
endif()
