cmake_minimum_required(VERSION 3.30)

include(helper_functions)
include(install_app_bible_workspace)

set(app_bible_workspace_VERSION_NUMBER 0.0)
set(app_bible_workspace_VERSION_NAME "0.0")

list(GET CMAKE_SYSTEM_LIBRARY_PATH 1 MINGW_LIB_DIRECTORY)
cmake_path(GET MINGW_LIB_DIRECTORY PARENT_PATH MINGW_ROOT_DIRECTORY)
message(STATUS "Found mingw64 directory: " ${MINGW_ROOT_DIRECTORY})
set(MINGW_SHARE_DIRECTORY "${MINGW_ROOT_DIRECTORY}/share")

project(
  app_bible_workspace
  VERSION ${app_bible_workspace_VERSION_NUMBER}
)

set(app_bible_workspace_EXE_NAME "app_bible_workspace")
set(app_bible_workspace_NAME "Bible Workspace")
set(app_bible_workspace_FILE_DESCRIPTION ${app_bible_workspace_NAME})
file(GLOB_RECURSE app_bible_workspace_CPP_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
file(GLOB_RECURSE app_bible_workspace_HPP_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp")

#
# Add executable.
#
add_executable(app_bible_workspace WIN32)

#
# Set custom executbale name.
#
set_target_properties(app_bible_workspace PROPERTIES OUTPUT_NAME ${app_bible_workspace_EXE_NAME})

#
# Include directories.
#
target_include_directories(app_bible_workspace
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Setup resource compiler.
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
# Copy files needed by binary
#
copy_folder(app_bible_workspace "${CMAKE_CURRENT_BINARY_DIR}/tessdata" "${MINGW_SHARE_DIRECTORY}/tessdata")

#
# Create Setup
#
set(TARGET_DEPLOY_DIR "${CMAKE_BINARY_DIR}/deploy/${app_bible_workspace_EXE_NAME}")
add_custom_target(app_bible_workspace_setup
  ALL
  DEPENDS
  app_bible_workspace
  COMMENT
  "Creates bible workspace setup."
)

#
# Copy executable, needed *.dll's and other files.
#
copy_target_files(app_bible_workspace_setup ${TARGET_DEPLOY_DIR} ${MINGW_ROOT_DIRECTORY})
copy_folder(app_bible_workspace_setup ${TARGET_DEPLOY_DIR}/tessdata ${MINGW_SHARE_DIRECTORY}/tessdata)

include(FindInnoSetup)

set(INNO_SETUP_REPO_BASE_PATH "${CMAKE_CURRENT_LIST_DIR}/..")
set(INNO_SETUP_APP_LONG_NAME ${app_bible_workspace_NAME})
set(INNO_SETUP_APP_EXE_NAME ${app_bible_workspace_EXE_NAME})

set(INNO_SETUP_APP_VERSION_NUMBER ${app_bible_workspace_VERSION_NUMBER})
set(INNO_SETUP_APP_VERSION_NAME ${app_bible_workspace_VERSION_NAME})
set(INNO_SETUP_APP_INSTALL_FOLDER "${INNO_SETUP_APP_LONG_NAME}")
set(INNO_SETUP_OUTPUT_NAME "setup_bible_workspace")
set(INNO_SETUP_OUTPUT_ICON "${CMAKE_CURRENT_LIST_DIR}/res/icon.ico")

generate_inno_setup(app_bible_workspace_setup app_bible_workspace ${CMAKE_CURRENT_LIST_DIR}/setup/setup.iss ${TARGET_DEPLOY_DIR})
