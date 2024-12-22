# Setup [WebUI](https://github.com/webui-dev/webui)

1.  clone git repository
2.  download headers from [here](https://github.com/webui-dev/webui/releases)
4.  Build **release** run `mingw32-make`
5.  Build **debug** run `mingw32-make debug`
6.  Optional secure mode build
    1.  run `pacman -S mingw-w64-x86_64-curl`
    2.  Build **release** run `mingw32-make WEBUI_USE_TLS=1 WEBUI_TLS_INCLUDE="C:\msys64\mingw64\include" WEBUI_TLS_LIB="C:\msys64\mingw64\lib"`
    3.  Build **debug** run `mingw32-make debug WEBUI_USE_TLS=1 WEBUI_TLS_INCLUDE="C:\msys64\mingw64\include" WEBUI_TLS_LIB="C:\msys64\mingw64\lib"`
