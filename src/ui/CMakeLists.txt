add_library(ui STATIC
    panels/ModelBrowserPanel.h
    panels/ModelBrowserPanel.cpp
    panels/PropertyPanel.h
    panels/PropertyPanel.cpp
    panels/LogPanel.h
    panels/LogPanel.cpp
    panels/RosMonitorPanel.h
    panels/RosMonitorPanel.cpp
    panels/PathDataPanel.h
    panels/PathDataPanel.cpp
    widgets/CollapsibleSection.h
    widgets/CollapsibleSection.cpp
)

target_link_libraries(ui core Qt5::Core Qt5::Widgets)
