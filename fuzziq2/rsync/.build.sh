#!/bin/bash -eu
# build.sh for OSS-Fuzz rsync project
# Builds rsync's core object files with instrumentation and links
# them against rsync_flist_fuzzer.c (recv_file_entry() harness).

cd "$SRC"/rsync

# --- Generate configure.sh / config.h.in from the git checkout -------------
# rsync's git repo ships without a generated configure script; the
# release tarballs do, but we're building from git in the Dockerfile,
# so this step is required. Needs autoconf/automake (installed in the
# Dockerfile) and one of python3-commonmark / python3-cmarkgfm for the
# manpage generation step -- if that's missing this will fail on the
# doc build, not the C build, so it's safe to patch around if it ever
# bites you (e.g. `touch` the generated manpage targets).
./prepare-source

# --- Configure --------------------------------------------------------------
# --disable-shared: we only want static objects to link into the fuzzer.
# --disable-xxhash/zstd/lz4/openssl: trims optional compression/crypto
# backends we don't need for flist parsing, and removes their libs from
# the dependency graph so the linker step stays simple. Drop these flags
# later if you want to fuzz code paths that touch those libraries.
CC="$CC" CFLAGS="$CFLAGS" ./configure \
    --disable-shared \
    --disable-xxhash \
    --disable-zstd \
    --disable-lz4 \
    --disable-openssl \
    --disable-md2man \
    --with-included-popt

# --- Build rsync's object files with instrumentation ------------------------
# We build the whole tree rather than hand-picking objects: recv_file_entry()
# transitively pulls in enough of rsync (filters, io, checksums, uid/gid
# lookups) that hand-listing objects is brittle across upstream changes.
# main.o is excluded at link time below, not here, since some of its
# helper statics are unrelated to main() and safer to just leave alone.
make -j"$(nproc)" 
#    CC="$CC" \
#    CFLAGS="$CFLAGS -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION"

# --- Build the fuzz target itself -------------------------------------------
$CC $CFLAGS -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION \
    -I"$SRC"/rsync -I"$SRC"/rsync/zlib \
    -c "$SRC"/rsync_flist_fuzzer.c -o "$SRC"/rsync/rsync_flist_fuzzer.o

# --- Link --------------------------------------------------------------------
# recv_file_entry() transitively touches globals/functions scattered across
# most of rsync's .c files (log.c's rprintf/rsyserr, options.c's am_server/
# who_am_i/protocol_version/..., cleanup.c's _exit_cleanup, syscall.c's
# do_stat/do_mkdir_at/..., lib/compat.c's strlcpy, etc). Hand-picking a
# subset of .o files is brittle -- it works until upstream adds a call
# path through a file you didn't list, then you get a wall of "undefined
# reference" errors instead of a helpful failure. So we link *every*
# object make produced, and exclude only the ones that define their own
# main() (main.o, plus any standalone test/support tools like wildtest.o
# or getgroups.o that some rsync versions build) since those would
# conflict with the main() supplied by $LIB_FUZZING_ENGINE.
RSYNC_OBJS=()
while IFS= read -r -d '' obj; do
    if nm "$obj" 2>/dev/null | grep -Eq '^[0-9a-f]+ T main$'; then
        continue
    fi
    RSYNC_OBJS+=("$obj")
done < <(find "$SRC"/rsync -maxdepth 2 -name '*.o' \
             ! -name 'rsync_flist_fuzzer.o' -print0)

$CXX $CXXFLAGS $LIB_FUZZING_ENGINE \
    "$SRC"/rsync/rsync_flist_fuzzer.o \
    "${RSYNC_OBJS[@]}" \
    -o "$OUT"/rsync_flist_fuzzer

# --- Seed corpus (optional but recommended) ----------------------------------
# A handful of real flist wire-format captures go a long way further
# than an empty corpus for a binary-protocol parser like this one.
# Drop any .bin captures you have into $SRC/flist_seeds/ in the
# Dockerfile and this will pick them up; otherwise this is a no-op.
if [ -d "$SRC/flist_seeds" ]; then
    zip -j "$OUT"/rsync_flist_fuzzer_seed_corpus.zip "$SRC"/flist_seeds/*
fi

# --- Options file -------------------------------------------------------------
# max_len keeps libFuzzer from wasting cycles on inputs far larger than
# a single file-list entry would realistically be on the wire.
cat > "$OUT"/rsync_flist_fuzzer.options << EOF
[libfuzzer]
max_len = 4096
EOF