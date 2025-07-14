cmake_minimum_required(VERSION 3.30)

project(bibstd)
add_library(bibstd OBJECT)

#
# Define an options
#
# noting yet

#
# Define target sources
#
file(GLOB BIBSTD_STD_SRC_FILES
  "${CMAKE_CURRENT_LIST_DIR}/*/*.cpp"

  # Include OS specific sources
  IF(WIN32)
  "${CMAKE_CURRENT_LIST_DIR}/*/windows/*.cpp"
  ENDIF()
)
file(GLOB BIBSTD_STD_INC_FILES
  "${CMAKE_CURRENT_LIST_DIR}/*/*.hpp"

  # Include OS specific headers
  IF(WIN32)
  "${CMAKE_CURRENT_LIST_DIR}/*/windows/*.hpp"
  ENDIF()
)

target_sources(bibstd
  PRIVATE ${BIBSTD_STD_SRC_FILES}
  PUBLIC ${BIBSTD_STD_INC_FILES}
)

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
target_include_directories(bibstd PUBLIC ${TESSERACT_INCLUDE_DIRS})
target_include_directories(bibstd PUBLIC ${LEPTONICA_INCLUDE_DIRS})

#
# Link libraries.
#
target_link_libraries(bibstd
  PUBLIC ${Boost_LIBRARIES}
  PUBLIC atomic
  PUBLIC fmt
  PUBLIC libs_external::all
  ${TESSERACT_LIBRARIES}
  ${LEPTONICA_LIBRARIES}
)

IF(WIN32)
  target_link_libraries(bibstd PUBLIC mswsock)
ENDIF()

#
# Target compile definitions.
#
IF(WIN32)
  target_compile_definitions(bibstd PUBLIC NOMINMAX)
ENDIF()

#
# Add target include dirs and target sources.
#
target_include_directories(bibstd PUBLIC ${CMAKE_CURRENT_LIST_DIR})
