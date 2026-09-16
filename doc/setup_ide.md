# Setup IDE (Windows)

1.  download and install Visual Studio Code [here](https://code.visualstudio.com/)
2.  download and install MSYS2 [here](https://www.msys2.org/)
3.  open `MSYS2 MSYS` (`msys2_shell.cmd`)
4.  run `pacman -Syu`
5.  run `pacman -S --needed base-devel mingw-w64-x86_64-toolchain`
6.  run `pacman -S mingw-w64-x86_64-cmake`
7.  run `pacman -S mingw-w64-x86_64-clang`
8.  open Visual Studio Code and install atleast
    - C/C++ Extension Pack [here](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack)
    - Clang-Format [here](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format)
    - Code Spell Checker

## Add mingw dependencies (Windows)

### Required

- **Boost** run `pacman -S mingw-w64-x86_64-boost` in `msys2_shell.cmd`
- **libzip** run `pacman -S mingw-w64-x86_64-libzip` in `msys2_shell.cmd`
- **Catch2** run `pacman -S mingw-w64-x86_64-catch` in `msys2_shell.cmd`
- **Curl** run `pacman -S mingw-w64-x86_64-curl` in `msys2_shell.cmd`
- **pugixml** run `pacman -S mingw-w64-x86_64-pugixml` in `msys2_shell.cmd`
- **Qt6 SVG** run `pacman -S mingw-w64-x86_64-qt6-svg` in `msys2_shell.cmd`
- **Qt6** run `pacman -S mingw-w64-x86_64-qt6-declarative` in `msys2_shell.cmd`
- **Spdlog** run `pacman -S mingw-w64-x86_64-spdlog` in `msys2_shell.cmd`
- **Tesseract Data (deu)** run `pacman -S mingw-w64-x86_64-tesseract-data-deu` in `msys2_shell.cmd`
- **Tesseract** run `pacman -S mingw-w64-x86_64-tesseract-ocr` in `msys2_shell.cmd`
- **WinRT** run `pacman -S mingw-w64-x86_64-cppwinrt` in `msys2_shell.cmd` (Windows only)

### Optional

- **Clang Tools Extra** run `pacman -S mingw-w64-x86_64-clang-tools-extra` in `msys2_shell.cmd`

## Deployment

Releases are packed by the release workflow, see the Release section of the README. Packing locally is only needed to test an installer:

- **.NET SDK 10** download and install
- **vpk** run `dotnet tool install --global vpk --version 1.2.0`, the version has to match `libs_external/velopack`

## Build

1.  cd to root directory
2.  run `mkdir build`
3.  run `cd build`
4.  run `cmake ..`
5.  run `cmake --build .`
