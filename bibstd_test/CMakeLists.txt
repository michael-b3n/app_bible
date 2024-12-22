cmake_minimum_required(VERSION 3.30)

project(bibstd_test LANGUAGES CXX)

find_package(Catch2 3 REQUIRED)

file(GLOB_RECURSE bibstd_test_CPP_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

#
# Set executable.
#
add_executable(bibstd_test)

#
# Set target sources.
#
target_sources(bibstd_test
  PRIVATE ${bibstd_test_CPP_FILES}
)

#
# Link libaries.
#
target_link_libraries(
  bibstd_test
  PUBLIC
  Catch2::Catch2WithMain
  bibstd)
