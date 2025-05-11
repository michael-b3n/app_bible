cmake_minimum_required(VERSION 3.30)

#
# Include cmake functions
#
include(cmake/copy_target_files.cmake)
include(helper_functions)
include(generate_inno_setup)
include(set_mingw_path_variables)

#
# Define app information
#
set(APP_VERSION_MAJOR 1)
set(APP_VERSION_MINOR 0)
set(APP_VERSION "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}")
set(APP_EXE_NAME "app_bible_workspace")
set(APP_NAME "Bible Workspace")
set(APP_FILE_DESCRIPTION ${APP_NAME})

#
# Configure version files
#
configure_file(${CMAKE_CURRENT_LIST_DIR}/res/resource.rc.in ${CMAKE_CURRENT_LIST_DIR}/res/resource.rc)
configure_file(${CMAKE_CURRENT_LIST_DIR}/res/version.hpp.in ${CMAKE_CURRENT_LIST_DIR}/src/version.hpp)

project(
  app_bible_workspace
  VERSION ${APP_VERSION}
)

file(GLOB_RECURSE app_bible_workspace_CPP_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
file(GLOB_RECURSE app_bible_workspace_HPP_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp")

#
# Add executable.
#
add_executable(app_bible_workspace WIN32)

#
# Set custom executbale name.
#
set_target_properties(app_bible_workspace PROPERTIES OUTPUT_NAME ${APP_EXE_NAME})

#
# Include directories.
#
target_include_directories(app_bible_workspace
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
target_sources(app_bible_workspace
  PRIVATE ${app_bible_workspace_CPP_FILES}
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/res/resource.rc
)

#
# Link against external dependencies
#
target_link_libraries(app_bible_workspace
  PRIVATE bibstd
)

#
# Find library directories
#
set_mingw_path_variables(MINGW_ROOT_DIRECTORY MINGW_SHARE_DIRECTORY)
message(STATUS "Found mingw64 root directory: " ${MINGW_ROOT_DIRECTORY})
message(STATUS "Found mingw64 share directory: " ${MINGW_SHARE_DIRECTORY})

#
# Copy files needed by binary
#
copy_folder(app_bible_workspace "${CMAKE_CURRENT_BINARY_DIR}/tessdata" "${MINGW_SHARE_DIRECTORY}/tessdata")

#
# Create Setup
#
set(TARGET_DEPLOY_DIR "${CMAKE_BINARY_DIR}/deploy/${APP_EXE_NAME}")
add_custom_target(app_bible_workspace_setup
  ALL
  DEPENDS
  app_bible_workspace
  COMMENT
  "create bible workspace setup"
)

#
# Copy executable, needed *.dll's and other files.
#
copy_target_files(app_bible_workspace_setup ${TARGET_DEPLOY_DIR} ${MINGW_ROOT_DIRECTORY})
copy_folder(app_bible_workspace_setup ${TARGET_DEPLOY_DIR}/tessdata ${MINGW_SHARE_DIRECTORY}/tessdata)

include(FindInnoSetup)

set(INNO_SETUP_REPO_BASE_PATH "${CMAKE_CURRENT_LIST_DIR}/..")
set(INNO_SETUP_APP_LONG_NAME ${APP_NAME})
set(INNO_SETUP_APP_EXE_NAME ${APP_EXE_NAME})

set(INNO_SETUP_APP_VERSION_NUMBER ${APP_VERSION})
set(INNO_SETUP_APP_VERSION_NAME ${APP_VERSION})
set(INNO_SETUP_APP_INSTALL_FOLDER "${INNO_SETUP_APP_LONG_NAME}")
set(INNO_SETUP_OUTPUT_NAME "setup_bible_workspace")
set(INNO_SETUP_OUTPUT_ICON "${CMAKE_CURRENT_LIST_DIR}/res/icon.ico")

generate_inno_setup(app_bible_workspace_setup app_bible_workspace ${CMAKE_CURRENT_LIST_DIR}/setup/setup.iss ${TARGET_DEPLOY_DIR})
