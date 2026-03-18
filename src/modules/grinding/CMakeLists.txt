add_library(mod_grinding STATIC
    GrindingModule.h
    GrindingModule.cpp
)

target_link_libraries(mod_grinding core Qt5::Core)
