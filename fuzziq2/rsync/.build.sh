#!/bin/bash
# build.sh - build the rsync exclude.c / parse_filter_str() fuzz target
#
# Verified against RsyncProject/rsync @ master (2026-08-18). This was
# developed and link-tested with gcc against a standalone test-main in an
# environment without clang; swap CC/CFLAGS below for your OSS-Fuzz /
# libFuzzer toolchain and it should link the same way, since the object
# list and stub layer don't depend on which compiler produced them.
#
# Usage:
#   1. Drop fuzz_parse_rule.c and fuzz_stub.c into the root of an rsync
#      checkout (NOT inside a directory mkproto.awk will scan differently --
#      see note below).
#   2. ./configure --disable-xxhash --disable-zstd --disable-lz4 \
#        --disable-openssl --disable-md2man
#   3. make proto.h
#   4. Run this script from the rsync checkout root.
#
# IMPORTANT: mkproto.awk (invoked by `make proto.h`) globs ./*.c to build
# proto.h's prototype list. If fuzz_parse_rule.c/fuzz_stub.c sit in that
# same directory when you run `make proto.h`, its naive parsing can corrupt
# proto.h (this bit us during development). Generate proto.h FIRST with the
# fuzz sources elsewhere, then copy them in afterward, as below.

set -eu

RSYNC_SRC="${1:-.}"
CC="${CC:-clang}"
CFLAGS="${CFLAGS:--O1 -fno-omit-frame-pointer -g -fsanitize=fuzzer,address}"
FUZZ_SRC_DIR="$(cd "$(dirname "$0")" && pwd)"

cd "$RSYNC_SRC"

# 1. Configure + generate proto.h/config.h/rounding.h BEFORE fuzz sources
#    are present in the tree (see note above).
if [ ! -f config.h ]; then
    ./configure --disable-xxhash --disable-zstd --disable-lz4 \
        --disable-openssl --disable-md2man
fi
[ -f proto.h ] || make proto.h

# 2. Build exclude.c plus the minimal dependency set (traced via
#    `nm -u exclude.o` and friends -- deliberately NOT the full
#    OBJS1/OBJS2/OBJS3 list from Makefile, which pulls in flist.c,
#    generator.c, io.c, checksum.c, etc. that exclude.c's rule parser
#    never touches).
DEPS="syscall.o util1.o util2.o \
      lib/wildmatch.o lib/compat.o lib/snprintf.o lib/permstring.o \
      lib/sysxattrs.o lib/pool_alloc.o lib/sysacls.o lib/acl.o \
      lib/mdfour.o lib/md5.o lib/md5-asm-x86_64.o"

make exclude.o $DEPS

# 3. Now bring in the fuzz driver + stub.
cp "$FUZZ_SRC_DIR/fuzz_parse_rule.c" .
cp "$FUZZ_SRC_DIR/fuzz_stub.c" .

$CC $CFLAGS -I. -I./popt -I./zlib -DHAVE_CONFIG_H -c fuzz_parse_rule.c -o fuzz_parse_rule.o
$CC $CFLAGS -I. -I./popt -I./zlib -DHAVE_CONFIG_H -c fuzz_stub.c -o fuzz_stub.o

# 4. Link. $LIB_FUZZING_ENGINE is a set of compiler/linker ARGUMENTS
#    supplied by the OSS-Fuzz build environment (e.g. "-fsanitize=fuzzer",
#    or a path to libFuzzingEngine.a) -- it goes on the command line, not
#    in front of it as if it were a program to exec. Outside OSS-Fuzz
#    (plain local libFuzzer build) it's unset, so fall back to
#    -fsanitize=fuzzer explicitly.
$CC $CFLAGS ${LIB_FUZZING_ENGINE:--fsanitize=fuzzer} -o "${OUT:-.}/fuzz_parse_rule" \
    fuzz_parse_rule.o fuzz_stub.o exclude.o $DEPS

echo "Built ${OUT:-.}/fuzz_parse_rule"
