# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(ImageNodeTool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts PrintSupport)


# OpenCV
find_package(OpenCV REQUIRED)

# QXlsx — 假设通过子目录或find_package引入
# 若使用add_subdirectory方式，将QXlsx源码放在third_party/QXlsx下
add_subdirectory(third_party/QXlsx)

# QCustomPlot — 编译为静态库
add_subdirectory(third_party/qcustomplot)

# 子模块
add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(nodes)
add_subdirectory(io)
add_subdirectory(gui)
add_subdirectory(cli)

# 主可执行文件
add_executable(ImageNodeTool main.cpp resources/resources.qrc)

target_include_directories(ImageNodeTool PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/QXlsx/header
)

target_link_libraries(ImageNodeTool PRIVATE
    common
    core
    nodes
    io
    gui
    cli
    QXlsx
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
)

# 拷贝资源目录到构建目录
file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})
