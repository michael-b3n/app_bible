# VerseLens

VerseLens is a Windows tray app that finds bible references on the screen. Point the cursor at a reference like "Johannes 3,16" in any window and press `ALT + f`: VerseLens reads the text around the cursor, recognizes the reference and opens it on [bibleserver.com](https://www.bibleserver.com). With the automatic search enabled, resting the cursor on a reference is enough.

## Install
Download `VerseLens-win-Setup.exe` from the latest [release](https://github.com/michael-b3n/biblia/releases/latest) and run it. VerseLens installs for the current user and starts at sign-in. New versions are downloaded in the background and installed on the next start, or right away through the update icon that appears next to the close button. Start at sign-in can be turned off in the Task Manager under Startup apps.

Uninstall versions 1.x ("Bible Assistant") first, they do not update to 2.x. Earlier 2.x versions were named ABA: their update should carry them over, otherwise uninstall ABA and install VerseLens again.

Requires Windows 10 or later.

## Scriptures
VerseLens ships without scriptures. To read passages in VerseLens, download USX bundles from the Digital Bible Library at [library.bible](https://library.bible/) and put the zip files into `%LOCALAPPDATA%\verselens\scriptures`. They are loaded on start, the folder can be changed in the settings.

## Repository
The app is the smaller part of this repository, the library below it holds the logic.

- `bibstd` — the library: bible references and their parsing (`bible`), the workflows that turn a hotkey into a lookup (`workflow`), OCR engines and text scripts (`txt`), scripture store and bibleserver lookup (`core`), threading and settings (`framework`), and a system layer whose Windows implementations sit in `system/windows`.
- `bibqml` — the Qt layer: the bridge between QML and the library, models and shared controls.
- `bibstd_test` — Catch2 tests of `bibstd`. Their scripture zips are local only, see `bibstd_test/res/scripture`.
- `verselens` — the app: window, tray, updater and the resources a release ships.
- `libs_external` — third party sources, used as they are.
- `tools` — CMake helpers, the clang-tidy runner and the MSIX packaging script.

## Development
In the MSYS2 MINGW64 shell:
```
cmake --preset gcc-release --fresh
cmake --build --preset gcc-release
ctest --preset gcc-release
cmake --install build
```
The presets `clang-debug`, `clang-release`, `gcc-debug` and `gcc-release` all build into `build`, `--fresh` replaces the configuration of the previous preset. CI and releases use `gcc-release`. The configure step downloads the prebuilt [Velopack](https://velopack.io) library, `cmake --install` fills `build/install`, the folder a release is packed from. Static analysis: `tools/run_clang_tidy.ps1`.

## Release
On the branch `release/verselens_v<major>`, set `APP_VERSION_MAJOR` and `APP_VERSION_MINOR` in `verselens/CMakeLists.txt`, then tag and push:
```
git tag verselens_v2.3
git push origin verselens_v2.3
```
The release workflow checks the tag, builds, tests and publishes the release. Installed apps pick it up within a day.

Once the repository variables `MSIX_IDENTITY_NAME`, `MSIX_PUBLISHER` and `MSIX_PUBLISHER_DISPLAY_NAME` hold the identity Partner Center shows, the workflow also packs an MSIX for the Microsoft Store and offers it as a build artifact, which is uploaded to Partner Center by hand. `tools/make_msix.ps1` packs the same package from an installed build locally. The store signs the package and delivers its updates, so an install from there runs without the Velopack updater.

## License
[MIT](LICENSE). The libraries in `libs_external` keep their own licenses, Qt is used under the LGPLv3, Tesseract and its `tessdata` under the Apache License 2.0. Scriptures are not part of this repository or of a release.
