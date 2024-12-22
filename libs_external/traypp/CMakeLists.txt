cmake_minimum_required(VERSION 3.30)
project(tray VERSION 0.2)

file(GLOB src
    "tray/src/*.cpp"
    "tray/src/*/*.cpp"
    "tray/src/*/*/*.cpp"
)

add_library(tray STATIC ${src})

target_include_directories(tray SYSTEM PUBLIC "tray/include")

target_compile_features(tray PRIVATE cxx_std_20)
set_target_properties(tray PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON)

set_target_properties(tray PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(tray PROPERTIES PROJECT_NAME ${PROJECT_NAME})

add_library(tray::tray ALIAS tray)
