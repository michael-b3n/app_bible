# ABA - Bible Assistant

ABA is a Windows tray app that finds bible references on the screen. Point the cursor at a reference like "Johannes 3,16" in any window and press `ALT + f`: ABA reads the text around the cursor, recognizes the reference and opens it on [bibleserver.com](https://www.bibleserver.com). With the automatic search enabled, resting the cursor on a reference is enough.

## Install
Download `ABA-win-Setup.exe` from the latest [release](https://github.com/michael-b3n/app_bible/releases/latest) and run it. ABA installs for the current user and starts at sign-in. New versions are downloaded in the background and installed on the next start, or right away through the update icon that appears next to the close button. Start at sign-in can be turned off in the Task Manager under Startup apps.

Uninstall versions 1.x ("Bible Assistant") first, they do not update to 2.x.

Requires Windows 10 or later.

## Scriptures
ABA ships without scriptures. To read passages in ABA, download USX bundles from the Digital Bible Library at [library.bible](https://library.bible/) and put the zip files into `%LOCALAPPDATA%\app_bible_assistant\scriptures`. They are loaded on start, the folder can be changed in the settings.

## Development
In the MSYS2 MINGW64 shell:
```
cmake --preset gcc-release
cmake --build --preset gcc-release
ctest --preset gcc-release
cmake --install build/gcc-release
```
The presets `clang-debug`, `clang-release`, `gcc-debug` and `gcc-release` build into `build/<preset>`, CI and releases use `gcc-release`. The configure step downloads the prebuilt [Velopack](https://velopack.io) library, `cmake --install` fills `build/<preset>/install`, the folder a release is packed from. Static analysis: `tools/run_clang_tidy.ps1`.

## Release
On the branch `release/aba_v<major>`, set `APP_VERSION_MAJOR` and `APP_VERSION_MINOR` in `aba/CMakeLists.txt`, then tag and push:
```
git tag aba_v2.1
git push origin aba_v2.1
```
The release workflow checks the tag, builds, tests and publishes the release. Installed apps pick it up within a day.

## License
[MIT](LICENSE). The libraries in `libs_external` keep their own licenses, Qt is used under the LGPLv3, Tesseract and its `tessdata` under the Apache License 2.0. Scriptures are not part of this repository or of a release.
