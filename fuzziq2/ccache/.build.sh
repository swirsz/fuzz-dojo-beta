#!/usr/bin/env bash
set -euo pipefail

# OSS-Fuzz mounts the project source at /src/<project_name>
PROJECT_DIR="/src/ccache"
BUILD_DIR="/tmp/ccache-build"
OUT_DIR="$OUT"

# Clean previous build artifacts
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# 1. Build ccache library & binaries (static linking preferred for fuzzing)
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_TESTING=OFF \
    -DENABLE_MANPAGE=OFF \
    -DENABLE_DOC=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_C_FLAGS="-fsanitize=fuzzer-no-link -g -O1" \
    -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer-no-link -g -O1"

cmake --build "$BUILD_DIR" -j$(nproc)

# 2. Compile the fuzzer target
# Adjust the path if your fuzzer is named differently
FUZZER_SRC="$PROJECT_DIR/fuzz/ccache_fuzzer.cc"
FUZZER_NAME="ccache_fuzzer"

# Link against the built ccache library and required system libs
clang++ "$FUZZER_SRC" \
    -I"$PROJECT_DIR/src" \
    -I"$PROJECT_DIR/build" \
    -L"$BUILD_DIR" \
    -lccache \
    -lz -llz4 -lzstd \
    -fsanitize=fuzzer \
    -g -O1 \
    -o "$OUT_DIR/$FUZZER_NAME"

# 3. Copy any required corpus files or dictionaries
# if [ -d "$PROJECT_DIR/fuzz/corpus" ]; then
#     cp -r "$PROJECT_DIR/fuzz/corpus" "$OUT_DIR/"
# fi

echo "Fuzz build completed successfully."
