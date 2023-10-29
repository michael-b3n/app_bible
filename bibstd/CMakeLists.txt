cmake_minimum_required(VERSION 3.26)

project(libbible)

#
# Include subdirectories
#
file(GLOB_RECURSE LIB_BIBLE_STD_SRC_FILES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB_RECURSE LIB_BIBLE_STD_INC_FILES "${CMAKE_CURRENT_LIST_DIR}/*.hpp")

#
# Create target
#
add_library(libbible OBJECT)

target_sources(libbible
  PRIVATE ${LIB_BIBLE_STD_SRC_FILES}
  PUBLIC ${LIB_BIBLE_STD_INC_FILES}
)

# find_package(Boost REQUIRED asio)
find_package(Boost REQUIRED COMPONENTS filesystem)
target_include_directories(libbible PUBLIC ${Boost_INCLUDE_DIRS})
find_package(spdlog REQUIRED)
target_include_directories(libbible INTERFACE ${spdlog_INCLUDE_DIRS})
find_package(fmt REQUIRED)
target_include_directories(libbible PUBLIC ${fmt_INCLUDE_DIRS})

#
# Link libraries.
#
target_link_libraries(libbible
  PUBLIC fmt
  PUBLIC Boost::filesystem
  PUBLIC libs_external::all
)

#
# Compile definitions.
#
target_compile_definitions(libbible PUBLIC
  NOMINMAX
)

#
# Add target include dirs and target sources.
#
target_include_directories(libbible PUBLIC ${CMAKE_CURRENT_LIST_DIR})
