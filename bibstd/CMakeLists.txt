cmake_minimum_required(VERSION 3.30)

project(bibstd)

#
# Include subdirectories
#
file(GLOB_RECURSE LIB_BIBLE_STD_SRC_FILES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB_RECURSE LIB_BIBLE_STD_INC_FILES "${CMAKE_CURRENT_LIST_DIR}/*.hpp")

#
# Create target
#
add_library(bibstd OBJECT)

target_sources(bibstd
  PRIVATE ${LIB_BIBLE_STD_SRC_FILES}
  PUBLIC ${LIB_BIBLE_STD_INC_FILES}
)

# find_package(Boost REQUIRED asio)
find_package(Boost REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
target_include_directories(bibstd PUBLIC ${Boost_INCLUDE_DIRS})
find_package(spdlog REQUIRED)
target_include_directories(bibstd INTERFACE ${spdlog_INCLUDE_DIRS})
find_package(fmt REQUIRED)
target_include_directories(bibstd PUBLIC ${fmt_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_search_module(TESSERACT REQUIRED tesseract)
pkg_search_module(LEPTONICA REQUIRED lept)
include_directories(${TESSERACT_INCLUDE_DIRS})
include_directories(${LEPTONICA_INCLUDE_DIRS})

#
# Link libraries.
#
target_link_libraries(bibstd
  PUBLIC atomic
  PUBLIC fmt
  PUBLIC ${Boost_LIBRARIES}
  PUBLIC libs_external::all
  ${TESSERACT_LIBRARIES}
  ${LEPTONICA_LIBRARIES}
)

IF(WIN32)
  target_link_libraries(bibstd
    PUBLIC mswsock
  )
ENDIF()

#
# Compile definitions.
#
target_compile_definitions(bibstd PUBLIC
  NOMINMAX
)

#
# Add target include dirs and target sources.
#
target_include_directories(bibstd PUBLIC ${CMAKE_CURRENT_LIST_DIR})
