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

target_link_libraries(mod_pathplan core Qt5::Core ${OpenCASCADE_LIBRARIES})
