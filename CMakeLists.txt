cmake_minimum_required(VERSION 3.16)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(MedianFilter)
endif()

add_library(median_filter INTERFACE)
target_include_directories(median_filter INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
