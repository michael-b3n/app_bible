cmake_minimum_required(VERSION 3.30)
project(magic_enum VERSION 0.9.6)

add_library(magic_enum INTERFACE)
target_include_directories(magic_enum INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(magic_enum::magic_enum ALIAS magic_enum)
