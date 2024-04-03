# Setup IDE (Windows)

1.  download and install Visual Studio Code [here](https://code.visualstudio.com/)
2.  download and install MSYS2 [here](https://www.msys2.org/)
3.  open `MSYS2 MSYS` (`msys2_shell.cmd`)
4.  run `pacman -Syu`
5.  run `pacman -S --needed base-devel mingw-w64-x86_64-toolchain`
6.  run `pacman -S mingw-w64-x86_64-cmake`
7.  run `pacman -S mingw-w64-x86_64-clang`
11. open Visual Studio Code and install atleast
    -   C/C++ Extension Pack [here](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack)
    -   Clang-Format [here](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format)
    -   Code Spell Checker

#### Add Dependencies

-  **OpenCV:** run `pacman -S mingw-w64-x86_64-opencv` in `msys2_shell.cmd`
-  **Boost** run `pacman -S mingw-w64-x86_64-boost` in `msys2_shell.cmd`
-  **Spdlog** run `pacman -S mingw-w64-x86_64-spdlog` in `msys2_shell.cmd`
-  **Fmt** run `pacman -S mingw-w64-x86_64-fmt` in `msys2_shell.cmd`
-  **Inno Setup** download and install [link](https://jrsoftware.org/isdl.php)

### Build

1.  cd to root directory
2.  run `mkdir build`
3.  run `cd build`
4.  run `cmake ..`
5.  run `cmake --build .`
