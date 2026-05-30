cmake_minimum_required(VERSION 4.3)

#
# Set global CMake variables.
#
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/tools/cmake")
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install")

#
# Limit CMake to only provide "Release" and "Debug" others are not needed.
# see https://cmake.org/cmake/help/latest/variable/CMAKE_CONFIGURATION_TYPES.html
#
set(CMAKE_CONFIGURATION_TYPES Release Debug)

project(app_bible)

#
#
#
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CSS_STANDARD_LIBRARIES}")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")

#
# Main Qt Versions
#
set(QT_VERSION_NAME Qt6)
set(QT_VERSION_NUMBER 6.11)

#
# Sets compile flag defaults.
#
add_compile_options(-Wall -Wpedantic -Werror)

#
# Force a CMake reconfiguration when the .git/index changes.
# This is needed to so the git-commit-hash is updated correctly.
#
set_property(GLOBAL APPEND
  PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/.git/index"
)

#
# Inlcude all subdirectories.
#
enable_testing()
add_subdirectory(libs_external)
add_subdirectory(bibstd)
add_subdirectory(bibqml)
add_subdirectory(bibstd_test)
add_subdirectory(app_bible_assistant)
