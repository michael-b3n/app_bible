#
# GenerateIncbin.cmake
#
# Function to generate INCBIN declarations for embedding binary files
#

#
# generate_incbin_cpp(
# TEMPLATE_FILE <path>          # Input template file (.in)
# OUTPUT_FILE <path>            # Output generated file (.cpp)
# FILES <file1> <file2> ...     # List of files to embed
# [RELATIVE_TO <path>]          # Optional: Make file paths relative to this directory
# )
#
# The function generates three CMake variables for use in the template:
# @INCBIN_DECLARATIONS@ - INC_RESOURCE declarations for all files
# @DATA_ARRAY@          - Array of data pointers (res_<label>_data)
# @SIZE_ARRAY@          - Array of size values (res_<label>_size)
# @FILE_COUNT@          - Number of files
#
function(generate_incbin)
  # Parse arguments
  cmake_parse_arguments(
    ARG # Prefix
    "" # Options (flags)
    "LABEL;RELATIVE_TO" # Single value args
    "FILES" # Multi-value args
    ${ARGN}
  )

  # Validate required arguments
  if(NOT ARG_LABEL)
    message(FATAL_ERROR "generate_incbin_cpp: LABEL is required")
  endif()

  if(ARG_RELATIVE_TO)
    if(NOT IS_ABSOLUTE "${ARG_RELATIVE_TO}")
      message(FATAL_ERROR "generate_incbin_cpp: RELATIVE_TO must be an absolute path: ${ARG_RELATIVE_TO}")
    endif()

    if(NOT IS_DIRECTORY "${ARG_RELATIVE_TO}")
      message(FATAL_ERROR "generate_incbin_cpp: RELATIVE_TO must be a valid directory: ${ARG_RELATIVE_TO}")
    endif()
  endif()

  # Initialize output variables
  set(INCBIN_DECLARATIONS_${ARG_LABEL} "")
  set(DATA_ARRAY_${ARG_LABEL} "")
  set(SIZE_ARRAY_${ARG_LABEL} "")
  list(LENGTH ARG_FILES FILE_COUNT_${ARG_LABEL})

  # Process each file
  foreach(FILE_PATH ${ARG_FILES})
    # Get the filename without extension for the label
    get_filename_component(FILE_BASENAME ${FILE_PATH} NAME_WE)

    # Sanitize the basename to create a valid C identifier (replace hyphens with underscores)
    string(REPLACE "-" "_" FILE_LABEL "${FILE_BASENAME}")

    # Determine the file path to use in INC_RESOURCE
    if(ARG_RELATIVE_TO)
      file(RELATIVE_PATH FILE_REL ${ARG_RELATIVE_TO} ${FILE_PATH})
    else()
      set(FILE_REL ${FILE_PATH})
    endif()

    # Add INCBIN declaration
    string(APPEND INCBIN_DECLARATIONS_${ARG_LABEL} "INC_RESOURCE(${FILE_LABEL}, \"${FILE_REL}\");\n")

    string(APPEND DATA_ARRAY_${ARG_LABEL} "res_${FILE_LABEL}_data, ")
    string(APPEND SIZE_ARRAY_${ARG_LABEL} "res_${FILE_LABEL}_size, ")
  endforeach()

  # Export variables to parent scope
  set(INCBIN_DECLARATIONS_${ARG_LABEL} "${INCBIN_DECLARATIONS_${ARG_LABEL}}" PARENT_SCOPE)
  set(DATA_ARRAY_${ARG_LABEL} "${DATA_ARRAY_${ARG_LABEL}}" PARENT_SCOPE)
  set(SIZE_ARRAY_${ARG_LABEL} "${SIZE_ARRAY_${ARG_LABEL}}" PARENT_SCOPE)
  set(FILE_COUNT_${ARG_LABEL} "${FILE_COUNT_${ARG_LABEL}}" PARENT_SCOPE)
endfunction()
