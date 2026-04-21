#!/bin/bash
# build.sh - Build script for AcheronARM64

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
BUILD_TYPE="${1:-Release}"

echo "=== AcheronARM64 Build Script ==="
echo "Build type: ${BUILD_TYPE}"
echo "Build directory: ${BUILD_DIR}"

# Configure
echo ""
echo "Configuring..."
cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DACHERONARM64_BUILD_TESTS=ON \
    -DACHERONARM64_BUILD_EXAMPLES=ON

# Build
echo ""
echo "Building..."
cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

# Test
echo ""
echo "Testing..."
ctest --test-dir "${BUILD_DIR}" --build-config "${BUILD_TYPE}" --output-on-failure

echo ""
echo "=== Build Complete ==="
echo "Binaries: ${BUILD_DIR}/"
echo "Tests: ${BUILD_DIR}/tests/"
echo "Examples: ${BUILD_DIR}/examples/"
