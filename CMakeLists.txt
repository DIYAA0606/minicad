cmake_minimum_required(VERSION 3.20)
project(MiniCAD VERSION 1.0.0 LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for language servers (clangd, ccls, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Compiler Warnings & Diagnostics
# ---------------------------------------------------------------------------
if(MSVC)
    add_compile_options(
        /W4
        /permissive-
        /Zc:__cplusplus
        /EHsc
    )
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
        -Wnon-virtual-dtor
    )
endif()

# ---------------------------------------------------------------------------
# Core MiniCAD Static Library
# ---------------------------------------------------------------------------
set(MINICAD_SOURCES
    src/geometry/Vector2D.cpp
    src/geometry/Circle.cpp
    src/geometry/Rectangle.cpp
    src/geometry/Polygon.cpp
    src/geometry/Intersection.cpp
    src/commands/TranslateCommand.cpp
    src/commands/RotateCommand.cpp
    src/commands/ScaleCommand.cpp
    src/commands/CommandHistory.cpp
    src/Scene.cpp
    src/SVGExporter.cpp
)

add_library(minicad_core STATIC ${MINICAD_SOURCES})

target_include_directories(minicad_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(minicad_core PUBLIC cxx_std_17)

# ---------------------------------------------------------------------------
# Demo Application
# ---------------------------------------------------------------------------
add_executable(minicad_demo examples/demo_scene.cpp)
target_link_libraries(minicad_demo PRIVATE minicad_core)

# ---------------------------------------------------------------------------
# Unit Testing with Catch2
# ---------------------------------------------------------------------------
option(BUILD_TESTING "Build unit tests" ON)

if(BUILD_TESTING)
    enable_testing()

    include(FetchContent)
    message(STATUS "Fetching Catch2 v3.5.3...")
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.3
    )
    # Exclude Catch2's internal tests and documentation to ensure fast compilation
    set(CATCH_BUILD_TESTING OFF CACHE INTERNAL "")
    set(CATCH_INSTALL_DOCS OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(Catch2)

    # Geometry test executable
    add_executable(test_geometry tests/test_geometry.cpp)
    target_link_libraries(test_geometry PRIVATE minicad_core Catch2::Catch2WithMain)
    add_test(NAME test_geometry COMMAND test_geometry)

    # Intersections & Geometric Algorithms test executable
    add_executable(test_intersections tests/test_intersections.cpp)
    target_link_libraries(test_intersections PRIVATE minicad_core Catch2::Catch2WithMain)
    add_test(NAME test_intersections COMMAND test_intersections)

    # Commands & Scene test executable
    add_executable(test_commands tests/test_commands.cpp)
    target_link_libraries(test_commands PRIVATE minicad_core Catch2::Catch2WithMain)
    add_test(NAME test_commands COMMAND test_commands)
endif()
