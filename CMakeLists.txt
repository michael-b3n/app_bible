cmake_minimum_required(VERSION 3.30)

#
# Add the folder tools/cmake  to the CMAKE_MODULE_PATH
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/tools/cmake")

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
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")

#
# Sets compile flag defaults.
#
add_compile_options(-Wall -Wpedantic -Werror)

#
# Add compile flag
#
# add_definitions(-DWINDOWS)
# add_definitions(-DMORE)

#
#
#
set(BIBLE_DEPLOY_BASE_DIR ${CMAKE_BINARY_DIR}/deploy)

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
add_subdirectory(bibstd_test)
add_subdirectory(app_bible_workspace)
