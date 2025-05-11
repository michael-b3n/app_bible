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
