# ============================================================
# src/CMakeLists.txt — MiniSTA core library and main executable
# ============================================================

# Core library (shared between Tcl shell and Python bindings)
add_library(minista_core STATIC
    core/sta_engine.cpp
    core/netlist_parser.cpp
)

# Required for Python shared library linking
set_target_properties(minista_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(minista_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(minista_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

# Find readline library
find_library(READLINE_LIBRARY readline)
find_path(READLINE_INCLUDE_DIR readline/readline.h)

# Main executable (Tcl shell)
add_executable(minista main.cpp)

target_include_directories(minista
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${READLINE_INCLUDE_DIR}
)

target_link_libraries(minista
    PRIVATE
        Tcl::tcl
        minista_core
        ${READLINE_LIBRARY}
)

target_compile_options(minista PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

# Install target
install(TARGETS minista DESTINATION bin)
