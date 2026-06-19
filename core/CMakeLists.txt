# core/CMakeLists.txt
add_library(core STATIC
    NodeBase.h NodeBase.cpp
    Connection.h Connection.cpp
    NodeFactory.h NodeFactory.cpp
    WorkflowGraph.h WorkflowGraph.cpp
    ExecutionEngine.h ExecutionEngine.cpp
    UndoCommands.h UndoCommands.cpp
)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core PUBLIC common Qt6::Core Qt6::Gui ${OpenCV_LIBS})
