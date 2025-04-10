include(helper_functions)

#
# Copy setup files.
#
function(copy_target_files target dest mingw)
  # Copy needed *.dll's.
  copy_file(${target} "${dest}" "${mingw}/bin/libboost_filesystem-mt.dll")
  copy_file(${target} "${dest}" "${mingw}/bin/libgcc_s_seh-1.dll")
  copy_file(${target} "${dest}" "${mingw}/bin/libstdc++-6.dll")
  copy_file(${target} "${dest}" "${mingw}/bin/libwinpthread-1.dll")
  # Copy executable.
  copy_file(${target} "${dest}" "${CMAKE_CURRENT_BINARY_DIR}/${app_bible_workspace_EXE_NAME}.exe")
endfunction(copy_target_files)
