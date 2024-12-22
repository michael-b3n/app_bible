cmake_minimum_required(VERSION 3.30)
project(incbin VERSION 1.0.0)

add_library(incbin INTERFACE)
target_include_directories(incbin INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(incbin::incbin ALIAS incbin)
