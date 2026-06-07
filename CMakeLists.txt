cmake_minimum_required(VERSION 3.16)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(MedianFilter CXX)
endif()

add_library(median_filter STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/MedianFilter.cpp"
)
target_include_directories(median_filter PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
