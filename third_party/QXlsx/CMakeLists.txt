file(GLOB QXLSX_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/header/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)

add_library(QXlsx STATIC ${QXLSX_SOURCES})

target_include_directories(QXlsx PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/header
)

# Qt6 private headers needed for qzipreader_p.h and qzipwriter_p.h
# Private headers use both <private/xxx.h> and <QtCore/private/xxx.h> includes
target_include_directories(QXlsx PRIVATE
    "D:/Qt/6.11.1/msvc2022_64/include/QtCore/6.11.1/QtCore"
    "D:/Qt/6.11.1/msvc2022_64/include/QtCore/6.11.1"
)

target_link_libraries(QXlsx PUBLIC
    Qt6::Core
    Qt6::Gui
)
