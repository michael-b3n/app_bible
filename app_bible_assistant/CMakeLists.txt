cmake_minimum_required(VERSION 3.30)

#
# Include cmake functions
#
include(helper_functions)
include(generate_inno_setup_generator_script)

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
# Define resources
#
set(TESSDATA_FOLDER_NAME "tessdata")
set(TESSDATA_FOLDER_PATH "${CMAKE_CURRENT_LIST_DIR}/res/${TESSDATA_FOLDER_NAME}")
message(STATUS "tessdata folder: " ${TESSDATA_FOLDER_PATH})

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

find_package(${QT_VERSION_NAME} ${QT_VERSION_NUMBER} REQUIRED COMPONENTS Quick)
qt_standard_project_setup(REQUIRES ${QT_VERSION_NUMBER})
qt_add_executable(${PROJECT_NAME} WIN32)

# add_subdirectory(qml)
qt_add_qml_module(
  ${PROJECT_NAME}
  URI "main"
  QML_FILES
  src/Main.qml
  IMPORTS
)
qt_generate_deploy_qml_app_script(
  TARGET ${PROJECT_NAME}
  OUTPUT_SCRIPT QT_DEPLOY_SCRIPT
  NO_UNSUPPORTED_PLATFORM_ERROR
  NO_TRANSLATIONS
  NO_COMPILER_RUNTIME
)

#
# Set custom executable name.
#
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${APP_EXE_NAME})

#
# Include directories.
#
target_include_directories(${PROJECT_NAME} PUBLIC ${Qt6_INCLUDE_DIRS})
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
  PRIVATE bibstd
)

#
# Configure installation
#
set_mingw_path(MINGW_ROOT_DIRECTORY MINGW_SHARE_DIRECTORY)
message(STATUS "found mingw64 root directory: " ${MINGW_ROOT_DIRECTORY})
message(STATUS "found mingw64 share directory: " ${MINGW_SHARE_DIRECTORY})

include(GNUInstallDirs)
install(CODE "file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX})")
install(
  TARGETS ${PROJECT_NAME}
  RUNTIME_DEPENDENCIES
  DIRECTORIES
  ${MINGW_ROOT_DIRECTORY}/bin
  PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
  POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
)
install(DIRECTORY ${TESSDATA_FOLDER_PATH} DESTINATION share)
install(SCRIPT ${QT_DEPLOY_SCRIPT})

#
# Create Inno Setup Executable
#
generate_inno_setup_generator_script(
  INNO_SETUP_GENERATOR_SCRIPT
  ${CMAKE_CURRENT_LIST_DIR}/setup/setup.iss
  ${CMAKE_CURRENT_LIST_DIR}/..
  ${CMAKE_CURRENT_LIST_DIR}/res/icon.ico)
install(SCRIPT "${INNO_SETUP_GENERATOR_SCRIPT}")
