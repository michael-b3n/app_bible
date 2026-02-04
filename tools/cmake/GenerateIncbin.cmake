#
# GenerateIncbin.cmake
#
# Function to generate INCBIN declarations for embedding binary files
#

#
# generate_incbin(
# LABEL <label>                 # Label prefix for generated variables
# FILES <file1> <file2> ...     # List of files to embed
# )
#
# The function generates three CMake variables for use in the template:
# @INCBIN_DECLARATIONS_${LABEL}@ - INC_RESOURCE declarations for all files
# @DATA_ARRAY_${LABEL}@          - Array of data pointers (res_<label>_data)
# @SIZE_ARRAY_${LABEL}@          - Array of size values (res_<label>_size)
# @FILE_COUNT_${LABEL}@          - Number of files
#
function(generate_incbin)
  # Parse arguments
  cmake_parse_arguments(
    ARG # Prefix
    "" # Options (flags)
    "LABEL" # Single value args
    "FILES" # Multi-value args
    ${ARGN}
  )

  # Validate required arguments
  if(NOT ARG_LABEL)
    message(FATAL_ERROR "generate_incbin: LABEL is required")
  endif()

  # Initialize output variables
  set(INCBIN_DECLARATIONS_${ARG_LABEL} "")
  set(FILENAME_ARRAY_${ARG_LABEL} "")
  set(DATA_ARRAY_${ARG_LABEL} "")
  set(SIZE_ARRAY_${ARG_LABEL} "")
  list(LENGTH ARG_FILES FILE_COUNT_${ARG_LABEL})

  # Process each file
  foreach(FILE_PATH ${ARG_FILES})
    # Get the filename without extension for the label
    get_filename_component(FILE_BASENAME ${FILE_PATH} NAME_WE)

    # Sanitize the basename to create a valid C identifier (replace hyphens with underscores)
    string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" FILE_LABEL "${FILE_BASENAME}")

    # Add INCBIN declaration
    string(APPEND INCBIN_DECLARATIONS_${ARG_LABEL} "INC_RESOURCE(${FILE_LABEL}, \"${FILE_PATH}\");\n")
    string(APPEND FILENAME_ARRAY_${ARG_LABEL} "\"${FILE_PATH}\", ")

    string(APPEND DATA_ARRAY_${ARG_LABEL} "res_${FILE_LABEL}_data, ")
    string(APPEND SIZE_ARRAY_${ARG_LABEL} "res_${FILE_LABEL}_size, ")
  endforeach()

  # Export variables to parent scope
  set(INCBIN_DECLARATIONS_${ARG_LABEL} "${INCBIN_DECLARATIONS_${ARG_LABEL}}" PARENT_SCOPE)
  set(FILENAME_ARRAY_${ARG_LABEL} "${FILENAME_ARRAY_${ARG_LABEL}}" PARENT_SCOPE)
  set(DATA_ARRAY_${ARG_LABEL} "${DATA_ARRAY_${ARG_LABEL}}" PARENT_SCOPE)
  set(SIZE_ARRAY_${ARG_LABEL} "${SIZE_ARRAY_${ARG_LABEL}}" PARENT_SCOPE)
  set(FILE_COUNT_${ARG_LABEL} "${FILE_COUNT_${ARG_LABEL}}" PARENT_SCOPE)
endfunction()
