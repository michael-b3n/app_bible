@if not defined _echo echo off

set VSWHERECOMMAND="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set ARGS=-version [17.0,18.0) -property installationPath -format value

rem No otherway to get return value of and command into a variable

echo VSWHERECOMMAND: %VSWHERECOMMAND%
echo ARGS: %ARGS%

%VSWHERECOMMAND% %ARGS% > .tmp_file_msvc_install_path.txt
set /p VWBASEPATH=<.tmp_file_msvc_install_path.txt
del .tmp_file_msvc_install_path.txt

echo VWBASEPATH %VWBASEPATH%

where cl >nul 2> nul
if not "%ERRORLEVEL%"=="0" (
  if exist "%VWBASEPATH%\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VWBASEPATH%\VC\Auxiliary\Build\vcvarsall.bat" amd64

    code ../
    exit 0
  )
)
