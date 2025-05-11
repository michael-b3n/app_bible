#
# Function to copy a folder as post_build command
# \param target the target on which post_build command shall be appended
# \param dst the destination folder
# \param folder the file that shall be copied
#
function(copy_folder target dst folder)
  add_custom_command(
    TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copy ${folder} to ${dst}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${folder} ${dst})
endfunction(copy_folder)

#
# Function to copy a file as post_build command
# \param target the target on which post_build command shall be appended
# \param dst the destination folder
# \param file the file that shall be copied
# \param optional new-file name
#
function(copy_file target dst file)
  if(DEFINED ARGV3)
    set(dst_file_name ${ARGV3})
  endif()

  add_custom_command(
    TARGET ${target} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E echo "Copy ${file} to ${dst}/${dst_file_name}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory ${dst}/
    COMMAND "${CMAKE_COMMAND}" -E copy ${file} ${dst}/${dst_file_name}
  )
endfunction(copy_file)

#
# Function to copy files as post_build command
# \param target the target on which post_build command shall be appended
# \param dst the destination folder
# \param src the source folder
#
function(copy_files target dst src)
  file(GLOB files "${src}/*")
  foreach(file ${files})
    copy_file(${target} ${dst} ${file})
  endforeach()
endfunction(copy_files)

#
# Set the mingw path variables.
# \param root directory (usually called MINGW_ROOT_DIRECTORY)
# \param share directory (usually called MINGW_SHARE_DIRECTORY)
#
function(set_mingw_path_variables mingw_root_dir mingw_share_dir)
  list(GET CMAKE_SYSTEM_LIBRARY_PATH 1 MINGW_LIB_DIRECTORY)
  cmake_path(GET MINGW_LIB_DIRECTORY PARENT_PATH MINGW_ROOT_DIRECTORY)
  set(${mingw_root_dir} "${MINGW_ROOT_DIRECTORY}" PARENT_SCOPE)
  set(${mingw_share_dir} "${MINGW_ROOT_DIRECTORY}/share" PARENT_SCOPE)
endfunction(set_mingw_path_variables)
