cmake_minimum_required(VERSION 4.3)
project(velopack VERSION 1.2.0)

#
# Select the library of the platform, checked before the download since the archive is about 30 MB.
# The archive ships libraries for windows, linux and macos, another platform adds its file here.
# Windows builds exist for MSVC only, MinGW links the C api of the dll directly.
# The dll records its name as velopack_libc.dll, the copy must keep that name to be found at runtime.
#
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VELOPACK_ARCHIVE_LIBRARY "velopack_libc_win_x64_msvc.dll")
  set(VELOPACK_LIBRARY "velopack_libc.dll")
else()
  message(FATAL_ERROR "velopack is not set up for this platform yet")
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

set(VELOPACK_LIBRARY_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin/${VELOPACK_LIBRARY}")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
file(COPY_FILE "${velopack_libc_SOURCE_DIR}/lib/${VELOPACK_ARCHIVE_LIBRARY}" "${VELOPACK_LIBRARY_PATH}" ONLY_IF_DIFFERENT)

add_library(velopack SHARED IMPORTED GLOBAL)
set_target_properties(velopack PROPERTIES
  IMPORTED_LOCATION "${VELOPACK_LIBRARY_PATH}"
  IMPORTED_IMPLIB "${VELOPACK_LIBRARY_PATH}"
)
target_include_directories(velopack SYSTEM INTERFACE "${velopack_libc_SOURCE_DIR}/include")

add_library(velopack::velopack ALIAS velopack)
