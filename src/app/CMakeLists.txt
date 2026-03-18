set(APP_SOURCES
    main.cpp
    MainWindow.h
    MainWindow.cpp
    PluginManager.h
    PluginManager.cpp
)

# 资源文件
qt5_add_resources(APP_RESOURCES ${CMAKE_SOURCE_DIR}/resources/resources.qrc)

add_executable(AIRobotGrinding ${APP_SOURCES} ${APP_RESOURCES})

target_link_libraries(AIRobotGrinding
    core
    ui
    mod_kinematics
    mod_cad
    mod_viewer
    mod_robot
    mod_pathplan
    mod_grinding
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
    ${OPENSCENEGRAPH_LIBRARIES}
    ${OpenCASCADE_LIBRARIES}
)

if(catkin_FOUND)
    target_link_libraries(AIRobotGrinding ${catkin_LIBRARIES})
endif()
