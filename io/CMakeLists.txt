# io/CMakeLists.txt
add_library(io STATIC
    WorkflowSerializer.h WorkflowSerializer.cpp
    ImageIO.h ImageIO.cpp
    DataIO.h DataIO.cpp
)
target_include_directories(io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/third_party/QXlsx/header)
target_link_libraries(io PUBLIC core common QXlsx Qt6::Core ${OpenCV_LIBS})
