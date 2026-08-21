#!/bin/bash -eu
# build.sh for OSS-Fuzz rsync project
# Builds rsync's core object files with instrumentation once, then links
# them against TWO harnesses:
#   - rsync_flist_fuzzer.c   -> recv_file_entry()   (flist.c)
#   - fuzz_parse_rule.c      -> parse_filter_str()  (exclude.c)
#
# Both harnesses need most of the same object graph (log.c, io.c,
# loadparm.c, util1/2.c, ...), so we build the tree once and link it
# twice rather than duplicating the whole-tree build step per-target.

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
# --disable-shared: we only want static objects to link into the fuzzers.
# --disable-xxhash/zstd/lz4/openssl: trims optional compression/crypto
# backends neither harness needs, and removes their libs from the
# dependency graph so the linker step stays simple. Drop these flags
# later if you want to fuzz code paths that touch those libraries.
# --disable-acl-support: acls.c/lib/sysacls.o call into the system's
# libacl (acl_get_entry, acl_set_file, ...), which we're not linking
# against. Neither recv_file_entry() nor parse_filter_str() needs ACL
# handling, so it's simpler to compile it out than to add -lacl to the
# link step. (An earlier, since-abandoned version of the exclude.c
# harness tried to work around this by hand-picking a minimal object
# list instead -- that turned out to be the more fragile path; letting
# configure remove ACL support upstream and linking the whole tree is
# more robust and is what both harnesses rely on now.)
CC="$CC" CFLAGS="$CFLAGS" ./configure \
    --disable-shared \
    --disable-xxhash \
    --disable-zstd \
    --disable-lz4 \
    --disable-openssl \
    --disable-md2man \
    --disable-acl-support \
    --with-included-popt

# --- Expose recv_file_entry() for the flist harness --------------------------
# recv_file_entry() is 'static' in flist.c upstream, giving it internal
# linkage. At -O1, with a single call site inside recv_file_list()'s loop,
# clang is free to inline it away completely and never emit an out-of-line
# body at all -- there's then no symbol in flist.o for a post-build
# `objcopy --globalize-symbol` to promote (verified: a single-call-site
# static function can vanish entirely at -O1). Dropping 'static' before
# compiling is the only way to guarantee an externally-linkable definition
# survives regardless of what the inliner decides to do at the call site.
#
# parse_filter_str() (the exclude.c target) needs no equivalent patch --
# it's already public in upstream rsync.
cp flist.c flist.c.orig
perl -0777 -pi -e 's/\bstatic(\s+struct\s+file_struct\s*\*\s*recv_file_entry\s*\()/$1/s' flist.c
if diff -q flist.c.orig flist.c > /dev/null; then
    echo "ERROR: failed to strip 'static' from recv_file_entry() in flist.c." >&2
    echo "The signature in this rsync checkout doesn't match the expected" >&2
    echo "pattern -- check 'grep -n recv_file_entry flist.c' and update the" >&2
    echo "perl substitution above in build.sh." >&2
    exit 1
fi
rm flist.c.orig

# --- Build rsync's object files with instrumentation ------------------------
# We build the whole tree rather than hand-picking objects for either
# target: both recv_file_entry() and parse_filter_str() transitively pull
# in enough of rsync (filters, io, checksums, uid/gid lookups, daemon
# config lookups, ...) that hand-listing objects per-harness is brittle
# across upstream changes and duplicates work between the two targets.
make -j"$(nproc)"

# --- Post-process objects so they're linkable into the fuzzers --------------
# main.c isn't just main() -- it also owns a chunk of process-global
# state that both harnesses' call graphs reach into (our_uid, our_gid,
# orig_umask, am_generator, am_receiver, local_server, daemon_connection,
# receive_sigusr2, sender_keeps_checksum, raw_argv/raw_argc, cooked_argv/
# cooked_argc, shell_exec, wait_process, client_run, start_server,
# remember_children, read/write_del_stats, batch_gen_fd, ...). We can't
# leave main.o out of either link, but its main() collides with the one
# $LIB_FUZZING_ENGINE supplies. Rename main() out of the way once here so
# the rest of the object stays linkable for both fuzz targets and the
# real main() is simply dead code in both fuzzer binaries.
objcopy --redefine-sym main=rsync_main_unused "$SRC"/rsync/main.o

# --- Build both fuzz targets -------------------------------------------------
$CC $CFLAGS -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION \
    -I"$SRC"/rsync -I"$SRC"/rsync/zlib \
    -c "$SRC"/rsync_flist_fuzzer.c -o "$SRC"/rsync/rsync_flist_fuzzer.o

$CC $CFLAGS -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION \
    -I"$SRC"/rsync -I"$SRC"/rsync/zlib \
    -c "$SRC"/fuzz_parse_rule.c -o "$SRC"/rsync/fuzz_parse_rule.o

# --- Link ---------------------------------------------------------------------
# Both recv_file_entry() and parse_filter_str() transitively touch globals/
# functions scattered across most of rsync's .c files (log.c's rprintf/
# rsyserr, options.c's am_server/who_am_i/protocol_version/..., cleanup.c's
# _exit_cleanup, syscall.c's do_stat/do_mkdir_at/..., loadparm.c's
# lp_use_chroot/lp_name/..., lib/compat.c's strlcpy, main.c's process-global
# state, etc). Hand-picking a subset of .o files per target is brittle --
# it works until upstream adds a call path through a file you didn't list,
# then you get a wall of "undefined reference" errors instead of a helpful
# failure. So we link *every* object make produced (main.o included -- it
# was patched above to no longer define main()) into *both* binaries, and
# only fall back to excluding an object here if it *still* defines a global
# main() after that patch, which would mean some other standalone test/
# support tool (e.g. wildtest.o, getgroups.o in some rsync versions) got
# built too and would otherwise collide with the main() supplied by
# $LIB_FUZZING_ENGINE.
#
# -Wl,--wrap=_exit_cleanup redirects every call to cleanup.c's real
# _exit_cleanup() (which calls exit() -- fatal to the whole fuzzer
# process, see both harnesses' comments) to each harness's own
# __wrap__exit_cleanup() instead, without touching cleanup.o at all. An
# earlier version of the flist harness tried an objcopy --redefine-sym
# rename on cleanup.o directly; that approach is more fragile (objcopy
# exits 0 and silently no-ops if the symbol name doesn't match exactly
# what's in the object, so a mismatch fails silently instead of loudly)
# and it did in fact fail silently in that environment. --wrap needs no
# exact symbol-table surgery, so prefer it for this kind of interception.
# Since it's a link-time flag rather than an object-file edit, both
# binaries can use it independently even though they share the same
# underlying cleanup.o.
RSYNC_OBJS=()
while IFS= read -r -d '' obj; do
    if nm "$obj" 2>/dev/null | grep -Eq '^[0-9a-f]+ T main$'; then
        continue
    fi
    RSYNC_OBJS+=("$obj")
done < <(find "$SRC"/rsync -maxdepth 2 -name '*.o' \
             ! -name 'rsync_flist_fuzzer.o' \
             ! -name 'fuzz_parse_rule.o' -print0)

$CXX $CXXFLAGS $LIB_FUZZING_ENGINE \
    -Wl,--wrap=_exit_cleanup \
    "$SRC"/rsync/rsync_flist_fuzzer.o \
    "${RSYNC_OBJS[@]}" \
    -o "$OUT"/rsync_flist_fuzzer

$CXX $CXXFLAGS $LIB_FUZZING_ENGINE \
    -Wl,--wrap=_exit_cleanup \
    "$SRC"/rsync/fuzz_parse_rule.o \
    "${RSYNC_OBJS[@]}" \
    -o "$OUT"/fuzz_parse_rule

# --- Seed corpora (optional but recommended) ---------------------------------
# A handful of real flist wire-format captures / real filter-rule strings
# go a long way further than an empty corpus for parsers like these.
# Drop captures into $SRC/flist_seeds/ and $SRC/filter_rule_seeds/ in the
# Dockerfile and this will pick them up; otherwise each is a no-op.
if [ -d "$SRC/flist_seeds" ]; then
    zip -j "$OUT"/rsync_flist_fuzzer_seed_corpus.zip "$SRC"/flist_seeds/*
fi
if [ -d "$SRC/filter_rule_seeds" ]; then
    zip -j "$OUT"/fuzz_parse_rule_seed_corpus.zip "$SRC"/filter_rule_seeds/*
fi

# --- Options files -------------------------------------------------------------
# max_len keeps libFuzzer from wasting cycles on inputs far larger than a
# single wire-format record (flist) or a realistic filter-rule string
# (exclude) would be.
cat > "$OUT"/rsync_flist_fuzzer.options << EOF
[libfuzzer]
max_len = 4096
EOF

cat > "$OUT"/fuzz_parse_rule.options << EOF
[libfuzzer]
max_len = 4096
EOF
