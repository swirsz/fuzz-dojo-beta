#!/bin/bash -eu
cd $SRC/rsync
./prepare-source
./configure --disable-shared
make -j$(nproc)

# Build fuzzer(s), linking against rsync object files that make sense
# for the target surface (e.g. checksum, flist parsing, protocol
# decoding — pick objects that don't pull in main()).
$CC $CFLAGS -Iproxy -I. -c $SRC/rsync_flist_fuzzer.c -o flist_fuzzer.o

$CXX $CXXFLAGS $LIB_FUZZING_ENGINE flist_fuzzer.o \
    <relevant .o files, e.g. flist.o util.o checksum.o ...> \
    -o $OUT/rsync_flist_fuzzer