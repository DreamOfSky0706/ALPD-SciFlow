# gui/CMakeLists.txt
# GUI 模块 —— SciFlow 完整图形界面
add_library(gui STATIC
    # Main window
    MainWindow.h
    MainWindow.cpp

    # Canvas scene and view
    NodeGraphicsScene.h
    NodeGraphicsScene.cpp
    NodeGraphicsView.h
    NodeGraphicsView.cpp

    # Node and port graphics items
    NodeGraphicsItem.h
    NodeGraphicsItem.cpp
    PortGraphicsItem.h
    PortGraphicsItem.cpp
    ConnectionGraphicsItem.h
    ConnectionGraphicsItem.cpp

    # Toolbox
    NodeToolbox.h
    NodeToolbox.cpp

    # Property panel
    PropertyPanel.h
    PropertyPanel.cpp
    FloatingParamPanel.h
    FloatingParamPanel.cpp

    # Preview and log
    PreviewPanel.h
    PreviewPanel.cpp
    LogPanel.h
    LogPanel.cpp

    # Custom editors
    CurveEditor.h
    CurveEditor.cpp
    GridEditor.h
    GridEditor.cpp
    GradientEditor.h
    GradientEditor.cpp
    AnnotationListEditor.h
    AnnotationListEditor.cpp
    FlowchartStepEditor.h
    FlowchartStepEditor.cpp
    TableEditor.h
    TableEditor.cpp
    LegendListEditor.h
    LegendListEditor.cpp
)

target_include_directories(gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

target_link_libraries(gui PUBLIC
    core
    io
    common
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
