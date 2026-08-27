cmake_minimum_required(VERSION 3.16)

project(app_framework VERSION 1.0 LANGUAGES CXX)

# Only force these settings when we're the top-level project.
# When consumed via add_subdirectory()/FetchContent, don't clobber
# the parent project's global compiler settings.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# Examples and tests are only built by default when this is the
# top-level project. A consumer pulling this in as a dependency
# won't build them unless explicitly requested.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(APP_FRAMEWORK_IS_TOP_LEVEL ON)
else()
    set(APP_FRAMEWORK_IS_TOP_LEVEL OFF)
endif()

option(APP_FRAMEWORK_BUILD_EXAMPLES "Build app_framework examples" ${APP_FRAMEWORK_IS_TOP_LEVEL})
option(APP_FRAMEWORK_BUILD_TESTS "Build app_framework tests" ${APP_FRAMEWORK_IS_TOP_LEVEL})

add_library(app_framework
    src/app.cpp
    src/framebuffer.cpp
)

# Namespaced alias so consumers always link as app_framework::app_framework,
# whether pulled in via add_subdirectory, FetchContent, or find_package.
add_library(app_framework::app_framework ALIAS app_framework)

set_target_properties(app_framework PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(app_framework PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
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

if(APP_FRAMEWORK_BUILD_EXAMPLES)
    file(GLOB EXAMPLE_SOURCES "examples/*.cpp")
    foreach(example_src ${EXAMPLE_SOURCES})
        get_filename_component(example_name ${example_src} NAME_WE)
        add_executable(${example_name} ${example_src})
        target_link_libraries(${example_name} PRIVATE app_framework::app_framework)
        if(WIN32)
            set_target_properties(${example_name} PROPERTIES WIN32_EXECUTABLE TRUE)
        endif()
    endforeach()
endif()

if(APP_FRAMEWORK_BUILD_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_executable(framebuffer_test tests/framebuffer_test.cpp)
        target_link_libraries(framebuffer_test PRIVATE app_framework::app_framework)
        add_test(NAME framebuffer_test COMMAND framebuffer_test)
    endif()
endif()

# ---------------------------------------------------------------------------
# Install rules / CMake package export (find_package, vcpkg, Conan support)
# ---------------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS app_framework
    EXPORT app_framework-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT app_framework-targets
    FILE app_framework-targets.cmake
    NAMESPACE app_framework::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/app_framework
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/app_framework-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/app_framework-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/app_framework-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/app_framework
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/app_framework-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/app_framework-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/app_framework
)