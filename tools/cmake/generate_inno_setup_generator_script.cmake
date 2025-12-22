function(generate_inno_setup_generator_script
  script_path
  inno_setup_template
  repo_base_path
  icon_path)
  set(INNO_SETUP_GENERATOR_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/inno_setup_generator_script.cmake")
  set(${script_path} ${INNO_SETUP_GENERATOR_SCRIPT} PARENT_SCOPE)
  find_package(InnoSetup REQUIRED)
  include(FindInnoSetup)

  file(WRITE ${INNO_SETUP_GENERATOR_SCRIPT}

    # ## Start of file write
    "
if(NOT \"${inno_setup_template}\" STREQUAL \"\")
  set(INNO_SETUP_TEMPLATE \"${inno_setup_template}\")
else()
  message(FATAL_ERROR \"invalid inno_setup_template argument\")
endif()

if(NOT \"${repo_base_path}\" STREQUAL \"\")
  set(INNO_SETUP_REPO_BASE_PATH \"${repo_base_path}\")
else()
  message(FATAL_ERROR \"invalid repo_base_path argument\")
endif()

if(NOT \"${icon_path}\" STREQUAL \"\")
  set(INNO_SETUP_OUTPUT_ICON \"${icon_path}\")
endif()

if(NOT \"${PROJECT_NAME}\" STREQUAL \"\")
  set(INNO_SETUP_TARGET \"${PROJECT_NAME}\")
else()
  message(FATAL_ERROR \"undefined PROJECT_NAME variable\")
endif()

if(NOT \"${APP_NAME}\" STREQUAL \"\")
  set(INNO_SETUP_APP_LONG_NAME \"${APP_NAME}\")
else()
  set(INNO_SETUP_APP_LONG_NAME \"${PROJECT_NAME}\")
  message(WARNING \"undefined APP_NAME variable\")
endif()

if(NOT \"${APP_EXE_NAME}\" STREQUAL \"\")
  set(INNO_SETUP_APP_EXE_NAME \"${APP_EXE_NAME}\")
else()
  set(INNO_SETUP_APP_EXE_NAME \"${PROJECT_NAME}\")
  message(WARNING \"undefined APP_EXE_NAME variable\")
endif()

if(NOT \"${APP_VERSION}\" STREQUAL \"\")
  set(INNO_SETUP_APP_VERSION_NUMBER \"${APP_VERSION}\")
  set(INNO_SETUP_APP_VERSION_NAME \"${APP_VERSION}\")
else()
  set(INNO_SETUP_APP_VERSION_NUMBER \"0.0\")
  set(INNO_SETUP_APP_VERSION_NAME \"0.0\")
  message(WARNING \"undefined APP_VERSION variable\")
endif()

if(NOT \"${APP_NAME}\" STREQUAL \"\")
  set(INNO_SETUP_APP_INSTALL_FOLDER \"${APP_NAME}\")
else()
  set(INNO_SETUP_APP_INSTALL_FOLDER \"${PROJECT_NAME}\")
  message(WARNING \"undefined APP_NAME variable\")
endif()

set(INNO_SETUP_OUTPUT_NAME \"setup_${PROJECT_NAME}\")

configure_file(\"${inno_setup_template}\" \"${CMAKE_INSTALL_PREFIX}/generated_setup.iss\")
execute_process(COMMAND \"${CMAKE_COMMAND}\" -E env \"${INNOSETUP_EXECUTABLE}\" \"${CMAKE_INSTALL_PREFIX}/generated_setup.iss\")
file(REMOVE \"${CMAKE_INSTALL_PREFIX}/generated_setup.iss\")"

    # End of file write
  )
  message(STATUS "created Inno Setup generator script: ${INNO_SETUP_GENERATOR_SCRIPT}")
endfunction(generate_inno_setup_generator_script)
