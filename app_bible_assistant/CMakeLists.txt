cmake_minimum_required(VERSION 3.30)

#
# Include cmake functions
#
include(cmake/copy_target_files.cmake)
include(helper_functions)
include(generate_inno_setup)

#
# Define app information
#
set(APP_VERSION_MAJOR 2)
set(APP_VERSION_MINOR 0)
set(APP_VERSION "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}")
set(APP_EXE_NAME "app_bible_assistant")
set(APP_NAME "Bible Assistant")
set(APP_FILE_DESCRIPTION ${APP_NAME})
message(STATUS "project name: " ${APP_EXE_NAME})
message(STATUS "project version: " ${APP_VERSION})

#
# Generate git information
#
generate_git_info(GIT_SHA1 GIT_DATE)
message(STATUS "git sha1: " ${GIT_SHA1})
message(STATUS "git date: " ${GIT_DATE})
if(GIT_SHA1 STREQUAL "")
  message(FATAL_ERROR "git sha1 not found")
endif()

#
# Configure version files
#
configure_file(${CMAKE_CURRENT_LIST_DIR}/res/resource.rc.in ${CMAKE_CURRENT_LIST_DIR}/res/resource.rc @ONLY)
configure_file(${CMAKE_CURRENT_LIST_DIR}/res/version.hpp.in ${CMAKE_CURRENT_LIST_DIR}/src/version.hpp @ONLY)

project(
  app_bible_assistant
  VERSION ${APP_VERSION}
)

file(GLOB_RECURSE app_bible_assistant_CPP_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
file(GLOB_RECURSE app_bible_assistant_HPP_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp")

#
# Add executable.
#
add_executable(app_bible_assistant WIN32)

#
# Set custom executbale name.
#
set_target_properties(app_bible_assistant PROPERTIES OUTPUT_NAME ${APP_EXE_NAME})

#
# Include directories.
#
target_include_directories(app_bible_assistant
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Setup resource compiler and file.
#
set(CMAKE_RC_COMPILER_INIT windres)
enable_language(RC)
set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")

#
# Set target sources.
#
target_sources(app_bible_assistant
  PRIVATE ${app_bible_assistant_CPP_FILES}
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/res/resource.rc
)

#
# Link against external dependencies
#
target_link_libraries(app_bible_assistant
  PRIVATE bibstd
)

#
# Find library directories
#
set_mingw_path(MINGW_ROOT_DIRECTORY MINGW_SHARE_DIRECTORY)
message(STATUS "found mingw64 root directory: " ${MINGW_ROOT_DIRECTORY})
message(STATUS "found mingw64 share directory: " ${MINGW_SHARE_DIRECTORY})

#
# Copy files needed by binary
#
copy_folder(app_bible_assistant "${CMAKE_CURRENT_BINARY_DIR}/tessdata" "${MINGW_SHARE_DIRECTORY}/tessdata")

#
# Create Setup
#
set(TARGET_DEPLOY_DIR "${CMAKE_BINARY_DIR}/deploy/${APP_EXE_NAME}")
add_custom_target(app_bible_assistant_setup
  ALL
  DEPENDS
  app_bible_assistant
  COMMENT
  "create bible assistant setup"
)

#
# Copy executable, needed *.dll's and other files.
#
copy_target_files(app_bible_assistant_setup ${TARGET_DEPLOY_DIR} ${MINGW_ROOT_DIRECTORY})
copy_folder(app_bible_assistant_setup ${TARGET_DEPLOY_DIR}/tessdata ${MINGW_SHARE_DIRECTORY}/tessdata)

include(FindInnoSetup)

set(INNO_SETUP_REPO_BASE_PATH "${CMAKE_CURRENT_LIST_DIR}/..")
set(INNO_SETUP_APP_LONG_NAME ${APP_NAME})
set(INNO_SETUP_APP_EXE_NAME ${APP_EXE_NAME})

set(INNO_SETUP_APP_VERSION_NUMBER ${APP_VERSION})
set(INNO_SETUP_APP_VERSION_NAME ${APP_VERSION})
set(INNO_SETUP_APP_INSTALL_FOLDER "${INNO_SETUP_APP_LONG_NAME}")
set(INNO_SETUP_OUTPUT_NAME "setup_bible_assistant")
set(INNO_SETUP_OUTPUT_ICON "${CMAKE_CURRENT_LIST_DIR}/res/icon.ico")

generate_inno_setup(app_bible_assistant_setup app_bible_assistant ${CMAKE_CURRENT_LIST_DIR}/setup/setup.iss ${TARGET_DEPLOY_DIR})
