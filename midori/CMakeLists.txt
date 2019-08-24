cmake_minimum_required (VERSION 3.11)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(coverage INTERFACE)
target_compile_options(coverage INTERFACE -O0 -g --coverage)
target_link_libraries(coverage INTERFACE --coverage)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
