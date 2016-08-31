cmake_minimum_required(VERSION 2.8.4)
project(flexLayout)

include_directories("${PROJECT_SOURCE_DIR}")
add_library(flexLayout flexLayout.h flexLayout.c)
