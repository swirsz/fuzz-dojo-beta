#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="${PROJECT_DIR:-/src/ccache}"
BUILD_DIR="${BUILD_DIR:-/tmp/ccache-build}"
OUT_DIR="${OUT:?OUT must point to the fuzzer output directory}"

CC_BIN="${CC:-clang}"
CXX_BIN="${CXX:-clang++}"

mkdir -p "$OUT_DIR"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# ccache uses C++20 facilities such as std::span. Verify that the compiler and
# its selected standard library actually provide <span>. Some Clang images
# default to an older libstdc++; libc++ may be available as an alternative.
check_span() {
    local extra_flag="$1"
    printf '#include <span>\nint main() { std::span<int> s; return (int)s.size(); }\n' \
        | "$CXX_BIN" -std=c++20 $extra_flag -x c++ - -c -o /tmp/ccache-span-probe.o \
            >/dev/null 2>&1
}

STDLIB_FLAG=""
if check_span ""; then
    :
elif check_span "-stdlib=libc++"; then
    STDLIB_FLAG="-stdlib=libc++"
else
    cat >&2 <<MSG
error: $CXX_BIN cannot compile a C++20 program that includes <span>.
Install/use a C++20-capable standard library (modern libstdc++ or libc++),
or point CC/CXX at a toolchain that provides one.
MSG
    exit 1
fi

BASE_C_FLAGS="${CFLAGS:-} -fsanitize=fuzzer-no-link -g -O1"
BASE_CXX_FLAGS="${CXXFLAGS:-} ${STDLIB_FLAG} -fsanitize=fuzzer-no-link -g -O1"

# Build the ccache framework with the same compiler and C++ standard library
# that will be used for the final libFuzzer executable.
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER="$CC_BIN" \
    -DCMAKE_CXX_COMPILER="$CXX_BIN" \
    -DENABLE_TESTING=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_CXX_EXTENSIONS=OFF \
    -DCMAKE_C_FLAGS="$BASE_C_FLAGS" \
    -DCMAKE_CXX_FLAGS="$BASE_CXX_FLAGS"

cmake --build "$BUILD_DIR" --parallel "$(nproc)"

FUZZER_SRC="$PROJECT_DIR/fuzz/ccache_fuzzer.cc"
FUZZER_NAME="ccache_fuzzer"
FRAMEWORK_LIB="$BUILD_DIR/src/ccache/libccache_framework.a"

[[ -f "$FUZZER_SRC" ]] || { echo "error: missing fuzzer source: $FUZZER_SRC" >&2; exit 1; }
[[ -f "$FRAMEWORK_LIB" ]] || { echo "error: missing framework library: $FRAMEWORK_LIB" >&2; exit 1; }

# Keep the final link on the exact same compiler/STL combination as CMake.
"$CXX_BIN" "$FUZZER_SRC" \
    -I"$PROJECT_DIR/src" \
    -I"$BUILD_DIR/src" \
    "$FRAMEWORK_LIB" \
    -L"$BUILD_DIR/src/third_party/fmt" \
    -L"$BUILD_DIR/src/third_party/blake3" \
    -L"$BUILD_DIR/src/third_party/cxxurl" \
    -L"$BUILD_DIR/_deps/hiredis-build" \
    -L"$BUILD_DIR/_deps/xxhash-build" \
    -ldep_fmt \
    -ldep_blake3 \
    -ldep_cxxurl \
    -ldep_hiredis \
    -ldep_xxhash \
    -lz -llz4 -lzstd -lpthread \
    $STDLIB_FLAG \
    ${CXXFLAGS:-} \
    -fsanitize=fuzzer \
    -std=c++20 \
    -g -O1 \
    -o "$OUT_DIR/$FUZZER_NAME"

echo "Fuzz build completed successfully: $OUT_DIR/$FUZZER_NAME"
