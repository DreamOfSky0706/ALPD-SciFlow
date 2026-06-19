# nodes/CMakeLists.txt
file(GLOB_RECURSE NODE_SOURCES
    "*.h"
    "*.cpp"
)

add_library(nodes STATIC ${NODE_SOURCES})

target_include_directories(nodes PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/image_io
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry
    ${CMAKE_CURRENT_SOURCE_DIR}/color
    ${CMAKE_CURRENT_SOURCE_DIR}/filter
    ${CMAKE_CURRENT_SOURCE_DIR}/morphology
    ${CMAKE_CURRENT_SOURCE_DIR}/edge
    ${CMAKE_CURRENT_SOURCE_DIR}/channel
    ${CMAKE_CURRENT_SOURCE_DIR}/composite
    ${CMAKE_CURRENT_SOURCE_DIR}/text
    ${CMAKE_CURRENT_SOURCE_DIR}/data_io
    ${CMAKE_CURRENT_SOURCE_DIR}/data_proc
    ${CMAKE_CURRENT_SOURCE_DIR}/chart
    ${CMAKE_CURRENT_SOURCE_DIR}/stylize
    ${CMAKE_CURRENT_SOURCE_DIR}/layout
    ${CMAKE_CURRENT_SOURCE_DIR}/drawing
    ${CMAKE_CURRENT_SOURCE_DIR}/utility
    ${CMAKE_CURRENT_SOURCE_DIR}/pattern
    ${CMAKE_CURRENT_SOURCE_DIR}/text_io
    ${CMAKE_SOURCE_DIR}/third_party/QXlsx/header
)

target_link_libraries(nodes PUBLIC core common QXlsx qcustomplot Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Charts ${OpenCV_LIBS})

