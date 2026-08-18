#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="/src/ccache"
BUILD_DIR="/tmp/ccache-build"
OUT_DIR="$OUT"

# Clean previous build artifacts
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# 1. Build ccache framework library
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_TESTING=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_CXX_EXTENSIONS=OFF \
    -DCMAKE_C_FLAGS="-fsanitize=fuzzer-no-link -g -O1" \
    -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer-no-link -g -O1"

cmake --build "$BUILD_DIR" -j$(nproc)

# 2. Compile the fuzzer target
FUZZER_SRC="$PROJECT_DIR/fuzz/ccache_fuzzer.cc"
FUZZER_NAME="ccache_fuzzer"

# Find the built framework library (adjust name if ccache changes it)
FRAMEWORK_LIB="$BUILD_DIR/src/ccache/libccache_framework.a"

# Link against ccache framework + dependencies
clang++ "$FUZZER_SRC" \
    -I"$PROJECT_DIR/src" \
    -I"$BUILD_DIR/src" \
    -L"$BUILD_DIR/src/ccache" \
    -L"$BUILD_DIR/src/third_party/fmt" \
    -L"$BUILD_DIR/src/third_party/blake3" \
    -L"$BUILD_DIR/src/third_party/cxxurl" \
    -L"$BUILD_DIR/_deps/hiredis-build" \
    -L"$BUILD_DIR/_deps/xxhash-build" \
    -lccache_framework \
    -ldep_fmt \
    -ldep_blake3 \
    -ldep_cxxurl \
    -ldep_hiredis \
    -ldep_xxhash \
    -lz -llz4 -lzstd -lpthread \
    -fsanitize=fuzzer \
    -std=c++20 \
    -g -O1 \
    -o "$OUT_DIR/$FUZZER_NAME"

echo "Fuzz build completed successfully."
