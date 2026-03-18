cmake_minimum_required(VERSION 3.16)
project(AIRobotGrinding VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ============================================================================
# 依赖查找
# ============================================================================
# ── macOS ──────────────────────────────────────────────────────────────────
if(APPLE)
    set(OpenCASCADE_DIR "$ENV{OpenCASCADE_DIR}")
    if(NOT DEFINED Qt5_DIR)
        foreach(_qt5_prefix
                "/opt/homebrew/opt/qt@5/lib/cmake/Qt5"
                "/usr/local/opt/qt@5/lib/cmake/Qt5")
            if(EXISTS "${_qt5_prefix}")
                set(Qt5_DIR "${_qt5_prefix}")
                break()
            endif()
        endforeach()
    endif()
endif()

if(APPLE)
    get_filename_component(_qt5_prefix "${Qt5_DIR}/../../.." ABSOLUTE)
    include_directories(BEFORE "${_qt5_prefix}/include")
endif()

# ── Windows ─────────────────────────────────────────────────────────────────
# 推荐用 vcpkg 统一安装依赖：
#   vcpkg install qt5-base qt5-opengl opencascade open-scene-graph orocos-kdl
# 然后 cmake 传入 -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
#
# 若手动安装，按实际路径设置下列变量（可在 cmake-gui 里设置）：
#   Qt5_DIR        = C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5
#   OpenCASCADE_DIR= C:/OpenCASCADE-7.7.0/cmake
#   OSG_DIR        = C:/OSG/lib/cmake/OpenSceneGraph
if(WIN32)
    # MSVC 需要定义 _USE_MATH_DEFINES 才能使用 M_PI
    add_definitions(-D_USE_MATH_DEFINES)
    # 关闭 MSVC 的 min/max 宏，避免与 std::min/max 冲突
    add_definitions(-DNOMINMAX)
    # 让 Windows.h 只包含常用部分，加快编译
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

find_package(OpenSceneGraph REQUIRED COMPONENTS osgViewer osgGA osgDB osgUtil osgText)
find_package(OpenCASCADE REQUIRED)
find_package(catkin QUIET COMPONENTS roscpp std_msgs geometry_msgs sensor_msgs)

# ============================================================================
# 全局包含路径
# ============================================================================
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${OpenCASCADE_INCLUDE_DIR}
)

if(catkin_FOUND)
    include_directories(${catkin_INCLUDE_DIRS})
    add_definitions(-DHAS_ROS)
endif()

# ============================================================================
# 子模块
# ============================================================================
add_subdirectory(src/core)
add_subdirectory(src/modules/kinematics)
add_subdirectory(src/modules/cad)
add_subdirectory(src/modules/viewer)
add_subdirectory(src/modules/robot)
add_subdirectory(src/modules/pathplan)
add_subdirectory(src/modules/grinding)
add_subdirectory(src/ui)
add_subdirectory(src/app)
