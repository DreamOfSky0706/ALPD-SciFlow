# third_party/qcustomplot/CMakeLists.txt
add_library(qcustomplot STATIC qcustomplot.h qcustomplot.cpp)

target_include_directories(qcustomplot PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# QCustomPlot 需要这个宏定义才能以库形式编译
target_compile_definitions(qcustomplot PUBLIC QCUSTOMPLOT_COMPILE_LIBRARY)

target_link_libraries(qcustomplot PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
)
