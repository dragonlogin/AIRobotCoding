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
# KDL support (macOS: brew install orocos-kdl, Ubuntu: apt install liborocos-kdl-dev)
# ============================================================================
find_package(orocos_kdl QUIET)

target_link_libraries(mod_kinematics Qt5::Core Qt5::Gui)
target_include_directories(mod_kinematics PUBLIC
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
)

if(orocos_kdl_FOUND)
    message(STATUS "KDL found: ${orocos_kdl_VERSION} — using KDL kinematics solver")
    find_package(Eigen3 REQUIRED)
    target_compile_definitions(mod_kinematics PUBLIC HAS_KDL)
    target_include_directories(mod_kinematics PRIVATE
        ${orocos_kdl_INCLUDE_DIRS}
        ${EIGEN3_INCLUDE_DIR}
    )
    target_link_libraries(mod_kinematics ${orocos_kdl_LIBRARIES} Eigen3::Eigen)
else()
    message(STATUS "KDL not found — falling back to built-in numerical IK")
    message(STATUS "  macOS: brew install orocos-kdl")
    message(STATUS "  Ubuntu: sudo apt install liborocos-kdl-dev")
endif()

# ============================================================================
# Future ROS/MoveIt extension point (Ubuntu only)
# ============================================================================
if(catkin_FOUND)
    # TODO: add MoveItKinematics implementation
    # target_compile_definitions(mod_kinematics PUBLIC HAS_ROS)
    # target_link_libraries(mod_kinematics ${catkin_LIBRARIES})
endif()
