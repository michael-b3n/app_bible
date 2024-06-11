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
    DEPENDS ${dummy_target_name}
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
# Function will create post_build the setup
#
function(generate_inno_setup target_setup target_app template_file folder)
  file(GLOB_RECURSE files_to_delete LIST_DIRECTORIES true "${folder}")

  if(files_to_delete)
    file(REMOVE_RECURSE ${files_to_delete})
  endif()

  configure_file(${template_file} "${CMAKE_CURRENT_BINARY_DIR}/generated_setup.iss")

  find_package(InnoSetup)
  add_custom_command(
    TARGET ${target_setup} POST_BUILD
    WORKING_DIRECTORY ${folder}
    COMMAND "${CMAKE_COMMAND}" -E echo "Setup generation for ${folder}."
    COMMAND "${CMAKE_COMMAND}" -E make_directory ${folder}
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/generated_setup.iss" ${folder}/
    COMMAND "${CMAKE_COMMAND}" -E env "${INNOSETUP_EXECUTABLE}" "${folder}/generated_setup.iss"
    COMMAND "${CMAKE_COMMAND}" -E remove "${folder}/generated_setup.iss"
    COMMENT "Generating Setup..."
  )
endfunction(generate_inno_setup)
