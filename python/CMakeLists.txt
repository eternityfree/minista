# ============================================================
# python/CMakeLists.txt — Python bindings for MiniSTA
# ============================================================

pybind11_add_module(minista_py bindings.cpp)

# Rename output to just "minista" for clean import
set_target_properties(minista_py PROPERTIES
    OUTPUT_NAME minista
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/python"
)

target_include_directories(minista_py
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(minista_py
    PRIVATE
        minista_core
)

target_compile_options(minista_py PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)
