/*
 * rsync_flist_fuzzer.c
 *
 * Simple libFuzzer driver for rsync's file-list entry parser
 * (recv_file_entry() in flist.c). This is the function that decodes
 * one file-list record off the wire when rsync receives a file list
 * from a peer -- historically a rich source of parsing bugs
 * (integer overflows in length fields, path traversal via crafted
 * names, etc.), which is why it's a good OSS-Fuzz target.
 *
 * recv_file_entry() reads from a file descriptor via rsync's io.c
 * layer (read_buf/read_varlong/etc.), not from an in-memory buffer,
 * so this harness backs that fd with a memfd populated from the
 * fuzz input. That lets us call the real, unmodified parsing code.
 *
 * Build notes (for build.sh):
 *   - build.sh links against every .o rsync's `make` produces (not a
 *     hand-picked subset) since recv_file_entry() transitively pulls in
 *     enough of the tree that a curated list is brittle across upstream
 *     changes. main.o's main() is renamed via objcopy so it doesn't
 *     collide with the one $LIB_FUZZING_ENGINE supplies; cleanup.o's
 *     _exit_cleanup() is left untouched and instead intercepted at link
 *     time via -Wl,--wrap=_exit_cleanup, which redirects calls to
 *     __wrap__exit_cleanup() below (see that function's comment for why
 *     --wrap rather than the same objcopy trick used for main.o).
 *   - recv_file_entry() is 'static' in flist.c upstream; build.sh patches
 *     that out with a source-level sed/perl step before compiling flist.c
 *     (a post-hoc `objcopy --globalize-symbol` isn't reliable here since
 *     -O1 can inline away a single-call-site static function entirely,
 *     leaving no symbol to promote).
 *   - rsync's rprintf()/rwrite() family goes to stderr in these
 *     builds; that's fine for fuzzing, just noisy -- consider
 *     redirecting stderr in run_fuzzer.sh if it's slowing you down.
 *   - protocol_version must be set to something recv_file_entry()
 *     supports before the first call; rsync.h externs it. Pin it to
 *     the latest supported protocol (check rsync.h's PROTOCOL_VERSION)
 *     unless you specifically want to fuzz older protocol paths too.
 */

/* memfd_create() is glibc-specific and gated behind _GNU_SOURCE; this
 * must be defined before any system header is pulled in (including
 * transitively via rsync.h), so it goes first in the file. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <setjmp.h>

#include "rsync.h"

#ifdef FUZZ_DOJO_TRACE
#include "fuzz_dojo_trace.h"
#endif

/* rsync.h does NOT centrally declare these as extern -- every .c file
 * in the tree (io.c, rsync.c, compat.c, clientserver.c, ...) repeats
 * its own local `extern int am_server;` etc. rather than relying on a
 * shared declaration, so the harness follows the same convention. */
extern int am_server;
extern int am_sender;
extern int protocol_version;

/* recv_file_entry() is 'static' in flist.c upstream; build.sh strips
 * that before compiling (see the comment at the top of this file). */
extern struct file_struct *recv_file_entry(int f, struct file_list *flist, int xflags);

static int make_input_fd(const uint8_t *data, size_t size) {
    int fd = memfd_create("flist_fuzz_input", 0);
    if (fd < 0) {
        return -1;
    }
    if (size > 0 && write(fd, data, size) != (ssize_t)size) {
        close(fd);
        return -1;
    }
    if (lseek(fd, 0, SEEK_SET) != 0) {
        close(fd);
        return -1;
    }
    return fd;
}

/* recv_file_entry() (via read_buf()/whine_about_eof() in io.c) calls
 * _exit_cleanup() -> exit() whenever it sees a premature EOF -- which,
 * for a fuzzer feeding it random/truncated bytes, is not a rare edge
 * case, it's most of the corpus. In the real rsync client/server that's
 * the right call: a broken protocol stream really is fatal. Inside
 * LLVMFuzzerTestOneInput() it's fatal to the *entire fuzzer process*,
 * which libFuzzer reports as "fuzz target exited" and then stops
 * running altogether, so it has to be intercepted rather than allowed
 * to run.
 *
 * build.sh links with -Wl,--wrap=_exit_cleanup, which redirects every
 * call to cleanup.c's real _exit_cleanup() to __wrap__exit_cleanup()
 * below instead -- no object-file surgery, no risk of a name mismatch
 * failing silently, no "multiple definition" conflict with the real
 * one (which is still present in cleanup.o, just no longer reachable
 * from any call site). None of the real one's cleanup logic (temp file
 * removal, reaping child processes, etc.) applies here -- there's no
 * real rsync session for it to clean up -- so we don't call through to
 * __real__exit_cleanup() at all, we just unwind back to the checkpoint
 * set in LLVMFuzzerTestOneInput() and treat this the same as a
 * rejected/malformed input.
 *
 * Not thread-safe: this global jmp_buf assumes libFuzzer's default
 * single-threaded mode. If you switch to -fork= or add real threading,
 * this needs to become thread-local. */
static jmp_buf fuzz_exit_jmp;

__attribute__((noreturn)) void __wrap__exit_cleanup(int code, const char *file, int line) {
    (void)code;
    (void)file;
    (void)line;
    longjmp(fuzz_exit_jmp, 1);
}

static int initialized = 0;

static void one_time_init(void) {
    if (initialized) {
        return;
    }
    /* Minimal global state recv_file_entry()/its callees expect.
     * Extend this if ASan/UBSan turns up an uninitialized-global read
     * that isn't a real bug -- that's a signal this list is incomplete,
     * not that the target code is broken. */
    protocol_version = PROTOCOL_VERSION;
    am_sender = 0;
    am_server = 1;
    initialized = 1;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    one_time_init();

    int fd = make_input_fd(data, size);
    if (fd < 0) {
        return 0;
    }

    if (setjmp(fuzz_exit_jmp) != 0) {
        /* Landed here via _exit_cleanup()'s longjmp -- recv_file_entry()
         * hit rsync's normal "protocol stream is broken" exit path.
         * Treat it like any other rejected malformed input. */
        close(fd);
        return 0;
    }

    struct file_list flist;
    memset(&flist, 0, sizeof(flist));

    struct file_struct *fs = recv_file_entry(fd, &flist, 0);

#ifdef FUZZ_DOJO_TRACE
    FUZZ_DOJO_TRACE_ARG_PTR("recv_file_entry_result", fs);
#endif

    /* recv_file_entry() allocates struct file_struct entries out of
     * flist's own pool allocator (see flist.c's file struct alloc
     * helpers) rather than plain malloc, so there's intentionally no
     * free(fs) here -- freeing it directly would corrupt the pool.
     * If you want leak-clean runs under LSan, call the matching
     * flist-teardown helper (e.g. flist_free()) on `flist` here once
     * you've confirmed which one your rsync revision uses. */

    close(fd);
    return 0;
}
