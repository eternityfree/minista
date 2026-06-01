cmake_minimum_required(VERSION 3.14)
project(MiniSTA
    VERSION 0.1.0
    DESCRIPTION "Lightweight Static Timing Analyzer with Tcl scripting"
    LANGUAGES C CXX
)

# ============================================================
# Global Settings
# ============================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories — binaries go to <project>/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================
# Options
# ============================================================
option(MINISTA_BUILD_TESTS "Build unit tests" ON)
option(MINISTA_BUILD_PYTHON "Build Python bindings" ON)

# ============================================================
# Include FetchContent module
# ============================================================
include(FetchContent)

# ============================================================
# Dependency: Tcl 8.6
# ============================================================
# Try to find system Tcl first. If not found, fall back to
# building from source via FetchContent.
# ============================================================

# Method 1: Try pkg-config
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(TCL QUIET tcl)
endif()

# Method 2: Try find_package
if(NOT TCL_FOUND)
    find_package(Tcl QUIET)
endif()

# Method 3: Manual search for common locations
if(NOT TCL_FOUND AND NOT TCL_INCLUDE_DIRS)
    find_path(TCL_INCLUDE_DIR
        NAMES tcl.h
        PATHS /usr/include/tcl8.6 /usr/include /usr/local/include
    )
    find_library(TCL_LIBRARY
        NAMES tcl8.6 tcl
        PATHS /usr/lib/x86_64-linux-gnu /usr/lib /usr/local/lib
    )
    if(TCL_INCLUDE_DIR AND TCL_LIBRARY)
        set(TCL_FOUND TRUE)
        set(TCL_INCLUDE_DIRS ${TCL_INCLUDE_DIR})
        set(TCL_LIBRARIES ${TCL_LIBRARY})
    endif()
endif()

# Create imported target for Tcl
if(TCL_FOUND)
    message(STATUS "Found system Tcl: ${TCL_LIBRARIES}")
    add_library(Tcl::tcl SHARED IMPORTED GLOBAL)
    set_target_properties(Tcl::tcl PROPERTIES
        IMPORTED_LOCATION ${TCL_LIBRARIES}
        INTERFACE_INCLUDE_DIRECTORIES "${TCL_INCLUDE_DIRS}"
    )
else()
    message(STATUS "System Tcl not found, using FetchContent...")
    # Fallback: FetchContent (slow, only if system Tcl not available)
    include(FetchContent)
    set(TCL_VERSION "8.6.14")
    FetchContent_Declare(
        tcl
        URL https://prdownloads.sourceforge.net/tcl/tcl${TCL_VERSION}-src.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(tcl)

    # Note: Building Tcl from source is complex. For development,
    # it's recommended to install tcl-dev package:
    #   sudo apt-get install tcl-dev    (Ubuntu/Debian)
    #   brew install tcl-tk             (macOS)
    message(FATAL_ERROR
        "Tcl development files not found. Please install:\n"
        "  Ubuntu/Debian: sudo apt-get install tcl-dev\n"
        "  macOS:         brew install tcl-tk\n"
        "  Fedora/RHEL:   sudo dnf install tcl-devel"
    )
endif()

# ============================================================
# Dependency: Google Test
# ============================================================
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://gitee.com/mirrors/googletest.git
    GIT_TAG        v1.14.0
)
# Prevent GTest from overriding compiler/linker options on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ============================================================
# Dependency: pybind11
# ============================================================
if(MINISTA_BUILD_PYTHON)
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://gitee.com/mirrors/pybind11.git
        GIT_TAG        v2.12.0
    )
    FetchContent_MakeAvailable(pybind11)
endif()

# ============================================================
# Project Targets
# ============================================================
add_subdirectory(src)

if(MINISTA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(MINISTA_BUILD_PYTHON)
    add_subdirectory(python)
endif()
