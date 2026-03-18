add_library(core STATIC
    PluginInterface.h
    EventBus.h
    EventBus.cpp
    DataModel.h
    DataModel.cpp
)

target_link_libraries(core PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)
