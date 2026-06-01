# ============================================================
# src/CMakeLists.txt — MiniSTA main executable
# ============================================================

# Collect source files (will grow as modules are implemented)
set(MINISTA_SOURCES
    main.cpp
)

# Create the main executable
add_executable(minista ${MINISTA_SOURCES})

# Set include paths
target_include_directories(minista
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(minista
    PRIVATE
        Tcl::tcl
)

# Compiler-specific options
target_compile_options(minista PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

# Install target
install(TARGETS minista DESTINATION bin)
