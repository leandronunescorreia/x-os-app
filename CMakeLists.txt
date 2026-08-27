cmake_minimum_required(VERSION 3.16)

project(app_framework VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(app_framework
    src/app.cpp
    src/framebuffer.cpp
)

set_target_properties(app_framework PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(app_framework PUBLIC include)
target_compile_features(app_framework PUBLIC cxx_std_17)

if(WIN32)
    target_sources(app_framework PRIVATE src/platform/win32_window.cpp)
    target_compile_definitions(app_framework PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
    target_link_libraries(app_framework PRIVATE user32 gdi32)
elseif(APPLE)
    enable_language(OBJCXX)
    target_sources(app_framework PRIVATE src/platform/cocoa_window.mm)
    target_link_libraries(app_framework PRIVATE "-framework Cocoa" "-framework QuartzCore")
elseif(UNIX)
    find_package(X11 REQUIRED)
    target_sources(app_framework PRIVATE src/platform/x11_window.cpp)
    target_link_libraries(app_framework PRIVATE X11::X11)
endif()

file(GLOB EXAMPLE_SOURCES "examples/*.cpp")
foreach(example_src ${EXAMPLE_SOURCES})
    get_filename_component(example_name ${example_src} NAME_WE)
    add_executable(${example_name} ${example_src})
    target_link_libraries(${example_name} PRIVATE app_framework)
    if(WIN32)
        set_target_properties(${example_name} PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
endforeach()

include(CTest)
if(BUILD_TESTING)
    add_executable(framebuffer_test tests/framebuffer_test.cpp)
    target_link_libraries(framebuffer_test PRIVATE app_framework)
    add_test(NAME framebuffer_test COMMAND framebuffer_test)
endif()
