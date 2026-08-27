# x-os-App Framework

A small C++17 windowed application framework with native backends for Win32, X11, and Cocoa.

## Public API

- `app_framework::App`: owns the lifecycle and event loop.
- `app_framework::Window`: platform-neutral window contract.
- `app_framework::MessageHandler`: receives normalized window messages.
- `app_framework::Framebuffer`: writable 32-bit BGRA pixel buffer.


## how to build?

## Library only
```sh
rm -rf build
cmake -B build -S . -DAPP_FRAMEWORK_BUILD_EXAMPLES=OFF -DAPP_FRAMEWORK_BUILD_TESTS=OFF
cmake --build build --target app_framework
```

## Library + tests only
```sh
rm -rf build && /
cmake -B build -S . -DAPP_FRAMEWORK_BUILD_EXAMPLES=OFF -DAPP_FRAMEWORK_BUILD_TESTS=ON && /-DBUILD_TESTING=ON
cmake --build build && /
ctest --test-dir build --output-on-failure
```

## Library + examples only
```sh
rm -rf build && /
cmake -B build -S . -DAPP_FRAMEWORK_BUILD_EXAMPLES=ON -DAPP_FRAMEWORK_BUILD_TESTS=OFF && /
cmake --build build
```

## Everything (your current default, made explicit)
```sh
rm -rf build && /
cmake -B build -S . -DAPP_FRAMEWORK_BUILD_EXAMPLES=ON -DAPP_FRAMEWORK_BUILD_TESTS=ON -DBUILD_TESTING=ON && /
cmake --build build && /
ctest --test-dir build --output-on-failure
```

## Release build, library only
```sh
rm -rf build && /
cmake -B build -S . -DAPP_FRAMEWORK_BUILD_EXAMPLES=OFF -DAPP_FRAMEWORK_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release && /
cmake --build build --config Release --target app_framework
```

## Visual Studio, library only
```sh
Remove-Item -Recurse -Force build && /
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DAPP_FRAMEWORK_BUILD_EXAMPLES=OFF -DAPP_FRAMEWORK_BUILD_TESTS=OFF && /
cmake --build build --config Release --target app_framework
```

## Visual Studio, library + tests
```sh
Remove-Item -Recurse -Force build && /
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DAPP_FRAMEWORK_BUILD_EXAMPLES=OFF -DAPP_FRAMEWORK_BUILD_TESTS=ON -DBUILD_TESTING=ON && /
cmake --build build --config Release && /
ctest --test-dir build --output-on-failure -C Release
```

## bonus install for linux
```sh
rm -rf build && \
cmake -B build -S . \
    -DAPP_FRAMEWORK_BUILD_EXAMPLES=OFF \
    -DAPP_FRAMEWORK_BUILD_TESTS=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$(pwd)/install-test && \
cmake --build build --config Release --target app_framework && \
cmake --install build --config Release
```


## bonus install for windows
```sh
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DAPP_FRAMEWORK_BUILD_EXAMPLES=OFF -DAPP_FRAMEWORK_BUILD_TESTS=OFF
cmake --build build --config Release --target app_framework
cmake --install build --config Release
```

Linux requires the X11 development package. The native backend is selected by CMake:

- Windows: `src/platform/win32_window.cpp`, linked with `user32` and `gdi32`.
- Linux: `src/platform/x11_window.cpp`, linked with Xlib.
- macOS: `src/platform/cocoa_window.mm`, linked with Cocoa and QuartzCore.

The `app_framework_example` target demonstrates creating a window, processing messages, drawing into the framebuffer, and presenting it.
