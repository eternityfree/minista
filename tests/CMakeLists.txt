# ============================================================
# tests/CMakeLists.txt — MiniSTA unit tests
# ============================================================

# Collect test source files
set(MINISTA_TEST_SOURCES
    test_main.cpp
)

# Create test executable
add_executable(minista_tests ${MINISTA_TEST_SOURCES})

# Set include paths
target_include_directories(minista_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(minista_tests
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        Tcl::tcl
)

# Compiler-specific options
target_compile_options(minista_tests PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(minista_tests)
