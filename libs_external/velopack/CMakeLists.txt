cmake_minimum_required(VERSION 4.3)
project(velopack VERSION 1.2.0)

# Checked before the download, the archive is about 30 MB.
if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "velopack is only set up for 64 bit windows")
endif()

#
# Velopack only ships prebuilt binaries, the release archive is downloaded instead of committed.
# see https://docs.velopack.io/getting-started/cpp
#
include(FetchContent)
FetchContent_Declare(
  velopack_libc
  URL "https://github.com/velopack/velopack/releases/download/${PROJECT_VERSION}/velopack_libc_${PROJECT_VERSION}.zip"
  URL_HASH SHA256=547262ed7a1ab1ff62f580aa53851ede2f1a451ac61b8974eb7bc01117488835
)
FetchContent_MakeAvailable(velopack_libc)

#
# Windows builds exist for MSVC only, MinGW links the C api of the dll directly.
# The dll records its name as velopack_libc.dll, the copy must keep that name to be found at runtime.
#
set(VELOPACK_DLL "${CMAKE_CURRENT_BINARY_DIR}/bin/velopack_libc.dll")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
file(COPY_FILE "${velopack_libc_SOURCE_DIR}/lib/velopack_libc_win_x64_msvc.dll" "${VELOPACK_DLL}" ONLY_IF_DIFFERENT)

add_library(velopack SHARED IMPORTED GLOBAL)
set_target_properties(velopack PROPERTIES
  IMPORTED_LOCATION "${VELOPACK_DLL}"
  IMPORTED_IMPLIB "${VELOPACK_DLL}"
)
target_include_directories(velopack SYSTEM INTERFACE "${velopack_libc_SOURCE_DIR}/include")

add_library(velopack::velopack ALIAS velopack)
