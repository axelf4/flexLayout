cmake_minimum_required(VERSION 2.8.11)
project(flexLayout)

add_library(flexLayout flexLayout.c)
target_include_directories(flexLayout PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
