cmake_minimum_required(VERSION 4.3)

include(GenerateIncbin)

project(bibstd)
add_library(${PROJECT_NAME} OBJECT)

#
# Configure incbin files
#
file(GLOB BIBSTD_DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/res/scripture/*.zip")
generate_incbin(LABEL "scripture" FILES "${BIBSTD_DATA_FILES}")
configure_file(${CMAKE_CURRENT_LIST_DIR}/res/scripture.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/res/scripture.hpp @ONLY)

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

target_sources(${PROJECT_NAME}
  PRIVATE ${BIBSTD_SRC_FILES}
  PUBLIC ${BIBSTD_INC_FILES}
)

find_package(Boost REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem url)
target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
find_package(spdlog REQUIRED)
target_include_directories(${PROJECT_NAME} INTERFACE ${spdlog_INCLUDE_DIRS})
find_package(pugixml REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${pugixml_INCLUDE_DIRS})
find_package(libzip REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${libzip_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_search_module(TESSERACT REQUIRED tesseract)
target_include_directories(${PROJECT_NAME} PUBLIC ${TESSERACT_INCLUDE_DIRS})
pkg_search_module(LEPTONICA REQUIRED lept)
target_include_directories(${PROJECT_NAME} PUBLIC ${LEPTONICA_INCLUDE_DIRS})
pkg_search_module(LIBCURL REQUIRED libcurl)
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBCURL_INCLUDE_DIRS})

#
# Link libraries.
#
target_link_libraries(${PROJECT_NAME}
  PUBLIC ${Boost_LIBRARIES}
  PUBLIC spdlog::spdlog
  PUBLIC pugixml
  PUBLIC libzip
  ${TESSERACT_LIBRARIES}
  ${LEPTONICA_LIBRARIES}
  ${LIBCURL_LIBRARIES}
  PUBLIC libs_external::all
  PUBLIC stdc++exp
)

IF(WIN32)
  target_link_libraries(${PROJECT_NAME} PUBLIC mswsock)
ENDIF()

#
# Target compile definitions.
#
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)

IF(WIN32)
  target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
ENDIF()

#
# Add target include dirs and target sources.
#
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR})
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH BIBSTD_INCLUDE_DIR)
target_include_directories(${PROJECT_NAME} PUBLIC ${BIBSTD_INCLUDE_DIR})
