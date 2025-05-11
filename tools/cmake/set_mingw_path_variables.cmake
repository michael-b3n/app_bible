#
# Set the mingw path variables
#
function(set_mingw_path_variables mingw_root_dir mingw_share_dir)
  list(GET CMAKE_SYSTEM_LIBRARY_PATH 1 MINGW_LIB_DIRECTORY)
  cmake_path(GET MINGW_LIB_DIRECTORY PARENT_PATH MINGW_ROOT_DIRECTORY)
  set(${mingw_root_dir} "${MINGW_ROOT_DIRECTORY}" PARENT_SCOPE)
  set(${mingw_share_dir} "${MINGW_ROOT_DIRECTORY}/share" PARENT_SCOPE)
endfunction(set_mingw_path_variables)
