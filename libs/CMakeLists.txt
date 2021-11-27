cmake_minimum_required (VERSION 3.8)

add_library(xbrz "xbrz/xbrz.cpp")

target_include_directories (xbrz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(xbrz PROPERTIES CXX_STANDARD 20)