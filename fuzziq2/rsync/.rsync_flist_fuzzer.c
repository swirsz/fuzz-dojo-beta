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
 *   - Link against flist.o, io.o, util.o, util2.o, checksum.o,
 *     exclude.o (filters), and whatever else the linker asks for --
 *     recv_file_entry() pulls in filter-list checks, uid/gid name
 *     lookups, and rprintf/logging plumbing from those files.
 *   - Do NOT link main.o (it defines main() and does argv parsing).
 *   - rsync's rprintf()/rwrite() family goes to stderr in these
 *     builds; that's fine for fuzzing, just noisy -- consider
 *     redirecting stderr in run_fuzzer.sh if it's slowing you down.
 *   - protocol_version must be set to something recv_file_entry()
 *     supports before the first call; rsync.h externs it. Pin it to
 *     the latest supported protocol (check rsync.h's PROTOCOL_VERSION)
 *     unless you specifically want to fuzz older protocol paths too.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "rsync.h"

#ifdef FUZZ_DOJO_TRACE
#include "fuzz_dojo_trace.h"
#endif

/* recv_file_entry() is declared 'static' in flist.c upstream. If your
 * instrumented copy hasn't had that relaxed, add a small shim in
 * flist.c (or a -D flag) to expose it, e.g.:
 *     #ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
 *     #define static
 *     #endif
 *   near the top of flist.c, matched by -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
 *   in build.sh (OSS-Fuzz sets this automatically for libFuzzer builds).
 */
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