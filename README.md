# App Bible Assistant

### Description
This app shall provide assistance when working with and studying in the bible. Currently this app features a basic OCR bible reference lookup on press of a keyboard shortcut `ALT + f` around the current cursor position. The bible reference is parsed and opened on [bibleserver.com](https://www.bibleserver.com) in the current browser.

### How to use?
1. Move your cursor above the bible reference you want to lookup.
2. Press `ALT + f`.
3. A new tab with the reference under the cursor is opened on [bibleserver.com](https://www.bibleserver.com).

### Supported OS
* Windows (Windows10+ is required)

### Supported OCR Languages
* German

More features, OS and language support are planned and in progress.

### Development
Configure and build:
```
cmake -S . -B build
cmake --build build
```

Run the unit tests, either directly or through CTest:
```
cmake --build build --target bibstd_test
./build/bibstd_test/bibstd_test
ctest --test-dir build --output-on-failure
```

Run the static analysis. It reads the compile flags from `build/compile_commands.json`, so the build has to be configured first. The checks are configured in `.clang-tidy`, with overrides for `bibqml` (Qt naming) and `bibstd_test`:
```
tools/run_clang_tidy.ps1
tools/run_clang_tidy.ps1 -Path bibstd/util
clang-tidy -p build bibstd/util/scope_guard.cpp
```
