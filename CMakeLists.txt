cmake_minimum_required(VERSION 3.16)
project(AIRobotGrinding VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ============================================================================
# Dependency resolution
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
# Recommended: use vcpkg to install all dependencies:
#   vcpkg install qt5-base qt5-opengl opencascade osg orocos-kdl
# Then pass to cmake: -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
#
# For manual installs, set these variables (e.g. via cmake-gui):
#   Qt5_DIR        = C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5
#   OpenCASCADE_DIR= C:/OpenCASCADE-7.7.0/cmake
#   OSG_DIR        = C:/OSG/lib/cmake/OpenSceneGraph
if(WIN32)
    # Required by MSVC to expose M_PI from <cmath>
    add_definitions(-D_USE_MATH_DEFINES)
    # Prevent Windows.h min/max macros from conflicting with std::min/std::max
    add_definitions(-DNOMINMAX)
    # Reduce Windows.h include size for faster builds
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    # Force MSVC to read/write source files as UTF-8
    if(MSVC)
        add_compile_options(/utf-8)
    endif()
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL)
find_package(Qt5LinguistTools QUIET)  # optional — needed only for .ts -> .qm compilation
find_package(OpenSceneGraph REQUIRED COMPONENTS osgViewer osgGA osgDB osgUtil osgText)
find_package(OpenCASCADE REQUIRED)
find_package(catkin QUIET COMPONENTS roscpp std_msgs geometry_msgs sensor_msgs)

# ============================================================================
# Output directories: all artifacts go to build/bin/<Config>
# ============================================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
# MSVC multi-config generators append Debug/Release subdirectories automatically
foreach(_cfg Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${_cfg} _CFG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_CFG} "${CMAKE_BINARY_DIR}/bin/${_cfg}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_CFG} "${CMAKE_BINARY_DIR}/bin/${_cfg}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_CFG} "${CMAKE_BINARY_DIR}/lib/${_cfg}")
endforeach()

# ============================================================================
# Global include paths
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
# Submodules
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