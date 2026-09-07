cmake_minimum_required(VERSION 4.3)

project(bibstd_test LANGUAGES CXX)

find_package(Catch2 3 REQUIRED)
include(Catch)

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
# Provide the scripture resource folder. The embedded scripture data cannot be reused here, because including
# `bibstd/res/scripture.hpp` emits the data itself and is therefore limited to a single translation unit.
#
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH bibstd_test_PARENT_DIR)
target_compile_definitions(bibstd_test
  PRIVATE BIBSTD_TEST_SCRIPTURE_DIR="${bibstd_test_PARENT_DIR}/bibstd/res/scripture"
)

#
# Provide the OCR image folder and the tessdata folder. Both are only needed by the one off capture
# that turns a screenshot into the OCR test data, see bibstd_test/res/ocr/README.md.
#
target_compile_definitions(bibstd_test
  PRIVATE BIBSTD_TEST_OCR_DIR="${CMAKE_CURRENT_LIST_DIR}/res/ocr"
  PRIVATE BIBSTD_TEST_TESSDATA_DIR="${bibstd_test_PARENT_DIR}/app_bible_assistant/res/tessdata"
)

#
# Link libaries.
#
target_link_libraries(
  bibstd_test
  PUBLIC
  Catch2::Catch2WithMain
  bibstd)

#
# Register the Catch2 test cases with CTest, so they can be run via `ctest` in the build directory.
#
catch_discover_tests(bibstd_test)
