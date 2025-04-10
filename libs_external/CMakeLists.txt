cmake_minimum_required(VERSION 3.30)

project(libs_external)

add_subdirectory(incbin)
add_subdirectory(magic_enum)
add_subdirectory(traypp)
# add_subdirectory(webui)

# set(WEBUI_DIR ${CMAKE_CURRENT_LIST_DIR}/webui)

add_library(libs_external_all INTERFACE IMPORTED GLOBAL)
target_link_libraries(libs_external_all
  INTERFACE
  incbin::incbin
  magic_enum::magic_enum
  tray::tray
  # webui::webui
)

add_library(libs_external::all ALIAS libs_external_all)
