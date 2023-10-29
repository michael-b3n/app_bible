cmake_minimum_required(VERSION 3.26)

project(libs_external)

add_subdirectory(webui)

set(WEBUI_DIR ${CMAKE_CURRENT_LIST_DIR}/webui)

add_library(libs_external_all INTERFACE IMPORTED GLOBAL)
target_link_libraries(libs_external_all
  INTERFACE
  webui::webui
)

add_library(libs_external::all ALIAS libs_external_all)
