add_library(mod_viewer STATIC
    ViewerModule.h
    ViewerModule.cpp
    OsgViewerWidget.h
    OsgViewerWidget.cpp

    OccToOsgConverter.h
    OccToOsgConverter.cpp
    FacePickHandler.h
    FacePickHandler.cpp
    PathVisualizer.h
    PathVisualizer.cpp
)

target_link_libraries(mod_viewer
    core
    Qt5::Core Qt5::Widgets Qt5::OpenGL
    ${OPENSCENEGRAPH_LIBRARIES}
    ${OpenCASCADE_LIBRARIES}
)
