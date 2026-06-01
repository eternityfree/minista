#!/bin/bash
# ============================================================
# build.sh — MiniSTA build script
# ============================================================
# Usage:
#   ./build.sh          # Build project (C++ + Python)
#   ./build.sh clean    # Clean build directory
#   ./build.sh test     # Build and run tests
#   ./build.sh rebuild  # Clean + build
#   ./build.sh python   # Build only Python bindings
#   ./build.sh help     # Show this help
# ============================================================

set -e  # Exit on error

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
BIN_DIR="${PROJECT_DIR}/bin"
PYTHON_BIN_DIR="${BIN_DIR}/python"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# ============================================================
# Functions
# ============================================================

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

do_clean() {
    print_info "Cleaning build directory..."
    rm -rf "${BUILD_DIR}"
    print_info "Done."
}

do_configure() {
    print_info "Configuring CMake..."
    cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}"
}

do_build() {
    print_info "Building MiniSTA..."
    cmake --build "${BUILD_DIR}" -j$(nproc)
    print_info "Build complete."
    print_info "  C++ binary:  ${BIN_DIR}/minista"
    print_info "  Python lib:  ${PYTHON_BIN_DIR}/minista.so"
}

do_build_python() {
    print_info "Building Python bindings only..."
    cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}" -DMINISTA_BUILD_TESTS=OFF
    cmake --build "${BUILD_DIR}" --target minista_py -j$(nproc)
    print_info "Python binding built: ${PYTHON_BIN_DIR}/minista.so"
}

do_test() {
    print_info "Running tests..."
    cd "${BUILD_DIR}" && ctest --output-on-failure
    print_info "All tests passed."
}

show_usage() {
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  (none)    Build project (C++ + Python)"
    echo "  clean     Remove build directory"
    echo "  test      Build and run tests"
    echo "  rebuild   Clean + build"
    echo "  python    Build Python bindings only"
    echo "  help      Show this help"
}

# ============================================================
# Main
# ============================================================

cd "${PROJECT_DIR}"

case "${1:-}" in
    clean)
        do_clean
        ;;
    test)
        do_configure
        do_build
        do_test
        ;;
    rebuild)
        do_clean
        do_configure
        do_build
        ;;
    python)
        do_build_python
        ;;
    help|--help|-h)
        show_usage
        ;;
    "")
        do_configure
        do_build
        ;;
    *)
        print_error "Unknown command: $1"
        show_usage
        exit 1
        ;;
esac
