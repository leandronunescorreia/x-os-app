# x-os-App Framework

A small C++17 windowed application framework with native backends for Win32, X11, and Cocoa.

## Public API

- `app_framework::App`: owns the lifecycle and event loop.
- `app_framework::Window`: platform-neutral window contract.
- `app_framework::MessageHandler`: receives normalized window messages.
- `app_framework::Framebuffer`: writable 32-bit BGRA pixel buffer.

## Build

```sh
rm -rf build
cmake -B build -S .
cmake --build build
```
# release build
```sh
rm -rf build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

## to use visual studio to build
```sh
cmd -> rmdir /s /q build
powershell -> Remove-Item -Recurse -Force build
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Linux requires the X11 development package. The native backend is selected by CMake:

- Windows: `src/platform/win32_window.cpp`, linked with `user32` and `gdi32`.
- Linux: `src/platform/x11_window.cpp`, linked with Xlib.
- macOS: `src/platform/cocoa_window.mm`, linked with Cocoa and QuartzCore.

The `app_framework_example` target demonstrates creating a window, processing messages, drawing into the framebuffer, and presenting it.
