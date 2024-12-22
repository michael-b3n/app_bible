cmake_minimum_required(VERSION 3.30)

project(webui)

add_library(webui STATIC IMPORTED GLOBAL)

set_target_properties(webui PROPERTIES IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/debug/libwebui-2-static.a")
set_target_properties(webui PROPERTIES IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/libwebui-2-static.a")
target_include_directories(webui INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

# find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)

target_link_libraries(webui
  INTERFACE ws2_32
  # INTERFACE OpenSSL::SSL
  # INTERFACE OpenSSL::Crypto
)
add_library(webui::webui ALIAS webui)
