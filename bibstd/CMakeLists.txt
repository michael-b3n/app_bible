cmake_minimum_required(VERSION 3.30)

include(GenerateIncbin)

project(bibstd)
add_library(bibstd OBJECT)

#
# Define paths
#
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH BIBSTD_INCLUDE_DIR)

#
# Configure incbin files
#
file(GLOB BIBSTD_DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/res/scripture/*.zip")
generate_incbin(LABEL "scripture_raw" RELATIVE_TO "${BIBSTD_INCLUDE_DIR}" FILES "${BIBSTD_DATA_FILES}")
configure_file(${CMAKE_CURRENT_LIST_DIR}/res/scripture.cpp.in ${CMAKE_CURRENT_LIST_DIR}/data/res_scripture.cpp @ONLY)

#
# Define target sources
#
file(GLOB BIBSTD_SRC_FILES
  "${CMAKE_CURRENT_LIST_DIR}/*/*.cpp"

  # Include OS specific sources
  IF(WIN32)
  "${CMAKE_CURRENT_LIST_DIR}/*/windows/*.cpp"
  ENDIF()
)
file(GLOB BIBSTD_INC_FILES
  "${CMAKE_CURRENT_LIST_DIR}/*/*.hpp"

  # Include OS specific headers
  IF(WIN32)
  "${CMAKE_CURRENT_LIST_DIR}/*/windows/*.hpp"
  ENDIF()
)

target_sources(bibstd
  PRIVATE ${BIBSTD_SRC_FILES}
  PUBLIC ${BIBSTD_INC_FILES}
)

find_package(Boost REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem url)
target_include_directories(bibstd PUBLIC ${Boost_INCLUDE_DIRS})
find_package(spdlog REQUIRED)
target_include_directories(bibstd INTERFACE ${spdlog_INCLUDE_DIRS})
find_package(pugixml REQUIRED)
target_include_directories(bibstd PUBLIC ${pugixml_INCLUDE_DIRS})
find_package(libzip REQUIRED)
target_include_directories(bibstd PUBLIC ${libzip_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_search_module(TESSERACT REQUIRED tesseract)
target_include_directories(bibstd PUBLIC ${TESSERACT_INCLUDE_DIRS})
pkg_search_module(LEPTONICA REQUIRED lept)
target_include_directories(bibstd PUBLIC ${LEPTONICA_INCLUDE_DIRS})
pkg_search_module(LIBCURL REQUIRED libcurl)
target_include_directories(bibstd PUBLIC ${LIBCURL_INCLUDE_DIRS})

#
# Link libraries.
#
target_link_libraries(bibstd
  PUBLIC ${Boost_LIBRARIES}
  PUBLIC spdlog::spdlog
  PUBLIC pugixml
  PUBLIC libzip
  ${TESSERACT_LIBRARIES}
  ${LEPTONICA_LIBRARIES}
  ${LIBCURL_LIBRARIES}
  PUBLIC libs_external::all
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
target_include_directories(bibstd PUBLIC ${BIBSTD_INCLUDE_DIR})
