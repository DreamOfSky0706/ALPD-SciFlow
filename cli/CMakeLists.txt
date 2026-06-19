# cli/CMakeLists.txt
add_library(cli STATIC
    CLIRunner.h CLIRunner.cpp
)
target_include_directories(cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cli PUBLIC core io nodes common Qt6::Core)
