add_library(mod_robot STATIC
    RobotModule.h
    RobotModule.cpp
    RosBridge.h
    RosBridge.cpp
)

target_link_libraries(mod_robot core Qt5::Core)

if(catkin_FOUND)
    target_link_libraries(mod_robot ${catkin_LIBRARIES})
endif()
