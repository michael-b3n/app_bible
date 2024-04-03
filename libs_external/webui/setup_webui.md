# Setup [WebUI](https://github.com/webui-dev/webui)

1.  clone git repository
2.  download [curl](https://curl.se/windows/) (to `C:/curl-win64-latest/`)
3.  go to webui root folder
4.  Build **release** run `mingw32-make WEBUI_USE_TLS=1 WEBUI_TLS_INCLUDE="C:\curl-win64-latest\curl-8.7.1_6-win64-mingw\include" WEBUI_TLS_LIB="C:\curl-win64-latest\curl-8.7.1_6-win64-mingw\lib"`
5.  Build **debug** run `mingw32-make debug WEBUI_USE_TLS=1 WEBUI_TLS_INCLUDE="C:\curl-win64-latest\curl-8.7.1_6-win64-mingw\include" WEBUI_TLS_LIB="C:\curl-win64-latest\curl-8.7.1_6-win64-mingw\lib"`

