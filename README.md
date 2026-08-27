# x-os-App Framework

A small C++17 windowed application framework with native backends for Win32, X11, and Cocoa.

## Public API

- `app_framework::App`: owns the lifecycle and event loop.
- `app_framework::Window`: platform-neutral window contract.
- `app_framework::MessageHandler`: receives normalized window messages.
- `app_framework::Framebuffer`: writable 32-bit BGRA pixel buffer.

## Build

```sh
cmake -B build -S .
cmake --build build
ctest --test-dir build --output-on-failure
```

Linux requires the X11 development package. The native backend is selected by CMake:

- Windows: `src/platform/win32_window.cpp`, linked with `user32` and `gdi32`.
- Linux: `src/platform/x11_window.cpp`, linked with Xlib.
- macOS: `src/platform/cocoa_window.mm`, linked with Cocoa and QuartzCore.

The `app_framework_example` target demonstrates creating a window, processing messages, drawing into the framebuffer, and presenting it.
