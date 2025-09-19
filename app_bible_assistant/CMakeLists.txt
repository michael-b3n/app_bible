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

#
# Define target files
#
file(GLOB_RECURSE app_bible_assistant_CPP_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
file(GLOB_RECURSE app_bible_assistant_HPP_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp")
file(GLOB_RECURSE app_bible_assistant_QML_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.qml")

#
# Setup Qt6 and executable
#
set(QT_VERSION_NAME Qt6)
set(QT_VERSION_NUMBER 6.9)

set(QT_FORCE_CMP0156_TO_VALUE NEW)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(${QT_VERSION_NAME} ${QT_VERSION_NUMBER} REQUIRED COMPONENTS Widgets Quick)
qt_standard_project_setup(REQUIRES ${QT_VERSION_NUMBER})

qt_add_executable(${PROJECT_NAME} WIN32)

target_include_directories(${PROJECT_NAME} PUBLIC ${Qt6_INCLUDE_DIRS})
qt_add_qml_module(${PROJECT_NAME}
  URI "module"
  QML_FILES
  src/main.qml
  SOURCES
  RESOURCES
)

#
# Set custom executable name.
#
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${APP_EXE_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC TRUE)
set_target_properties(${PROJECT_NAME} PROPERTIES AUTORCC TRUE)
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOUIC TRUE)

#
# Include directories.
#
target_include_directories(${PROJECT_NAME}
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
target_sources(${PROJECT_NAME}
  PRIVATE ${app_bible_assistant_CPP_FILES}
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/res/resource.rc
)

#
# Link against external dependencies
#
target_link_libraries(${PROJECT_NAME}
  PRIVATE Qt6::Quick
  PRIVATE Qt6::Widgets
  PRIVATE bibstd
)

#
# Find library directories
#
set_mingw_path(MINGW_ROOT_DIRECTORY MINGW_SHARE_DIRECTORY)
message(STATUS "found mingw64 root directory: " ${MINGW_ROOT_DIRECTORY})
message(STATUS "found mingw64 share directory: " ${MINGW_SHARE_DIRECTORY})

#
# Configure tessdata folder
#
set(TESSDATA_VERSION "best") # other option would be "fast"
set(TESSDATA_FOLDER "${CMAKE_CURRENT_LIST_DIR}/res/tessdata-${TESSDATA_VERSION}")
message(STATUS "set tessdata folder: " ${TESSDATA_FOLDER})

#
# Copy files needed by binary
#
copy_folder(${PROJECT_NAME} "${CMAKE_CURRENT_BINARY_DIR}/tessdata" "${TESSDATA_FOLDER}")

#
# Create Setup
#
set(TARGET_DEPLOY_DIR "${CMAKE_BINARY_DIR}/deploy/${APP_EXE_NAME}")
add_custom_target(app_bible_assistant_setup
  ALL
  DEPENDS
  ${PROJECT_NAME}
  COMMENT
  "create ${PROJECT_NAME} setup"
)

#
# Copy executable, needed *.dll's and other files.
#
copy_target_files(${PROJECT_NAME}_setup ${TARGET_DEPLOY_DIR} ${MINGW_ROOT_DIRECTORY})
copy_folder(${PROJECT_NAME}_setup ${TARGET_DEPLOY_DIR}/tessdata ${TESSDATA_FOLDER})

include(FindInnoSetup)

set(INNO_SETUP_REPO_BASE_PATH "${CMAKE_CURRENT_LIST_DIR}/..")
set(INNO_SETUP_APP_LONG_NAME ${APP_NAME})
set(INNO_SETUP_APP_EXE_NAME ${APP_EXE_NAME})

set(INNO_SETUP_APP_VERSION_NUMBER ${APP_VERSION})
set(INNO_SETUP_APP_VERSION_NAME ${APP_VERSION})
set(INNO_SETUP_APP_INSTALL_FOLDER "${INNO_SETUP_APP_LONG_NAME}")
set(INNO_SETUP_OUTPUT_NAME "setup_${PROJECT_NAME}")
set(INNO_SETUP_OUTPUT_ICON "${CMAKE_CURRENT_LIST_DIR}/res/icon.ico")

generate_inno_setup(${PROJECT_NAME}_setup ${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/setup/setup.iss ${TARGET_DEPLOY_DIR})
