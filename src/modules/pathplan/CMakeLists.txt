add_library(mod_pathplan STATIC
    PathPlanModule.h
    PathPlanModule.cpp
)

target_link_libraries(mod_pathplan core Qt5::Core)
