#
# Function to add the resources given after \p source, which compiles them in via INC_RESOURCE using incbin.
# The object file of the source is rebuilt whenever one of its resources changes.
# Relative paths are resolved relative to the current source directory.
#
function(add_incbin_resources source)
  cmake_path(ABSOLUTE_PATH source BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} NORMALIZE OUTPUT_VARIABLE source_path)
  if(NOT EXISTS ${source_path})
    message(FATAL_ERROR "add_incbin_resources: source file not found: ${source_path}")
  endif()

  if(NOT ARGN)
    message(FATAL_ERROR "add_incbin_resources: no resources given: ${source_path}")
  endif()

  unset(resource_paths)
  foreach(resource ${ARGN})
    cmake_path(ABSOLUTE_PATH resource BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} NORMALIZE OUTPUT_VARIABLE resource_path)
    if(NOT EXISTS ${resource_path})
      message(FATAL_ERROR "add_incbin_resources: resource file not found: ${resource_path}")
    endif()
    list(APPEND resource_paths ${resource_path})
  endforeach()

  message(STATUS "incbin resources of ${source}: ${ARGN}")
  set_property(SOURCE ${source_path} APPEND PROPERTY OBJECT_DEPENDS ${resource_paths})
endfunction(add_incbin_resources)

#
# Set the mingw root path variable named by \p mingw_root_dir, the parent of the compiler's bin folder.
#
function(set_mingw_path mingw_root_dir)
  cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH MINGW_BIN_DIRECTORY)
  cmake_path(GET MINGW_BIN_DIRECTORY PARENT_PATH MINGW_ROOT_DIRECTORY)
  set(${mingw_root_dir} "${MINGW_ROOT_DIRECTORY}" PARENT_SCOPE)
endfunction(set_mingw_path)

#
# Set the git information variables named by \p git_sha1 and \p git_date.
#
function(generate_git_info git_sha1 git_date)
  find_package(Git REQUIRED)

  # the commit's SHA1, and whether the building assistant was dirty or not
  execute_process(COMMAND
    ${GIT_EXECUTABLE} describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_SHA1
    COMMAND_ERROR_IS_FATAL ANY OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(${git_sha1} "${GIT_SHA1}" PARENT_SCOPE)

  # the date of the commit
  execute_process(COMMAND
    ${GIT_EXECUTABLE} log -1 --format=%ad --date=local
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_DATE
    COMMAND_ERROR_IS_FATAL ANY OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(${git_date} "${GIT_DATE}" PARENT_SCOPE)
endfunction(generate_git_info)
