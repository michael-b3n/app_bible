cmake_minimum_required(VERSION 4.3)

project(rapidcsv VERSION 9.7.0)

add_library(rapidcsv INTERFACE)

#
# Marked as SYSTEM to keep the strict project warning flags (-Wall -Wpedantic -Werror)
# from being applied to the third party header.
#
target_include_directories(rapidcsv SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(rapidcsv::rapidcsv ALIAS rapidcsv)
