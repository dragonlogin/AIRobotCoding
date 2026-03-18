add_library(mod_pathplan STATIC
    PathPlanModule.h
    PathPlanModule.cpp
    GrindingPathGenerator.h
    GrindingPathGenerator.cpp
    PathOptimizer.h
    PathOptimizer.cpp
    PathSimulator.h
    PathSimulator.cpp
)

target_link_libraries(mod_pathplan core mod_kinematics Qt5::Core ${OpenCASCADE_LIBRARIES})
