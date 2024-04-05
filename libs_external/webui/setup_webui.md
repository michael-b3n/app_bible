# Setup [WebUI](https://github.com/webui-dev/webui)

1.  clone git repository
2.  download headers from [here](https://github.com/webui-dev/webui/releases)
3.  run `pacman -S mingw-w64-x86_64-curl`
4.  Build **release** run `mingw32-make WEBUI_USE_TLS=1 WEBUI_TLS_INCLUDE="C:\msys64\mingw64\include" WEBUI_TLS_LIB="C:\msys64\mingw64\lib"`
5.  Build **debug** run `mingw32-make debug WEBUI_USE_TLS=1 WEBUI_TLS_INCLUDE="C:\msys64\mingw64\include" WEBUI_TLS_LIB="C:\msys64\mingw64\lib"`
