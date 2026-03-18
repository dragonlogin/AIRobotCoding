add_library(core STATIC
    PluginInterface.h
    EventBus.h
    EventBus.cpp
    DataModel.h
    DataModel.cpp
)

target_link_libraries(core Qt5::Core)
