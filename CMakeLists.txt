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
#set(OpenCASCADE_DIR "/opt/opencascade/lib/cmake/opencascade")
if(APPLE)
    set(OpenCASCADE_DIR "$ENV{OpenCASCADE_DIR}")
    # Homebrew Qt5 路径（与 Qt6 共存时需要显式指定）
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
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL)

if(APPLE)
    # Qt6 installs headers into /usr/local/include/QtGui which shadows Qt5 framework
    # headers when included as <QtGui/header.h>. Force Qt5 include path first.
    get_filename_component(_qt5_prefix "${Qt5_DIR}/../../.." ABSOLUTE)
    include_directories(BEFORE "${_qt5_prefix}/include")
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
