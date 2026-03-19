# Compile .ts -> .qm (requires Qt5LinguistTools; skipped if not available)
set(TS_FILES ${CMAKE_SOURCE_DIR}/resources/translations/zh_CN.ts)
if(Qt5LinguistTools_FOUND)
    qt5_add_translation(QM_FILES ${TS_FILES})
    add_custom_target(translations ALL DEPENDS ${QM_FILES})
endif()

set(APP_SOURCES
    main.cpp
    MainWindow.h
    MainWindow.cpp
    PluginManager.h
    PluginManager.cpp
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
)

add_executable(AIRobotGrinding ${APP_SOURCES})

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

# Copy .qm files to translations/ subdirectory next to the executable
foreach(_qm ${QM_FILES})
    add_custom_command(TARGET AIRobotGrinding POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:AIRobotGrinding>/translations"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_qm}"
            "$<TARGET_FILE_DIR:AIRobotGrinding>/translations/"
    )
endforeach()
