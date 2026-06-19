# common/CMakeLists.txt
add_library(common STATIC
    Exceptions.h
    DataType.h
    PortDefinition.h
    NodeData.h NodeData.cpp
    DataTable.h DataTable.cpp
    Logger.h Logger.cpp
    Utility.h Utility.cpp
)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common PUBLIC Qt6::Core Qt6::Gui ${OpenCV_LIBS})
