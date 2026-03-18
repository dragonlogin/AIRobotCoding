add_library(mod_cad STATIC
    CadModule.h
    CadModule.cpp
)

target_link_libraries(mod_cad core Qt5::Core ${OpenCASCADE_LIBRARIES})
