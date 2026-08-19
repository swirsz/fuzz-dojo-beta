/*
 * fuzz_stub.c
 *
 * Minimal support layer for fuzzing rsync's exclude.c in isolation.
 * Modeled on rsync's own t_stub.c (used by their `tls` test helper), but
 * with the two symbols that t_stub.c defines and exclude.c ALSO defines
 * (check_filter, daemon_filter_list) removed, plus the additional globals
 * and io.c functions exclude.c references but this fuzz target never
 * actually exercises (filter-list transmission over the rsync protocol,
 * CVS-ignore defaults, etc.). None of these code paths are reachable from
 * parse_filter_str() on a plain in-memory rule string -- they only need to
 * resolve at link time.
 */
#include "rsync.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>

/* exclude.c calls exit_cleanup() on fatal parse errors (overlong merge-file
 * names, I/O errors opening a merge file, etc). A real exit() would kill
 * the whole (long-lived) libFuzzer process on the first such input instead
 * of just rejecting that one input, tanking throughput and coverage. We
 * longjmp back to the top of LLVMFuzzerTestOneInput() instead. */
jmp_buf fuzz_exit_jmp;

/* --- plain global state, all "off"/default --- */
int do_fsync = 0;
int inplace = 0;
int am_daemon = 0;
int am_chrooted = 0;
int insecure_links = 0;
int modify_window = 0;
int preallocate_files = 0;
int sparse_files = 0;
int protect_args = 0;
int module_id = -1;
int relative_paths = 0;
unsigned int module_dirlen = 0;
int preserve_xattrs = 0;
int preserve_perms = 0;
int preserve_executability = 0;
int omit_link_times = 0;
int open_noatime = 0;
size_t max_alloc = (size_t)-1;
char *partial_dir;
char *module_dir;
int module_dirfd = -1;
char *confine_root;
unsigned int confine_rootlen = 0;

int recurse = 0;
int xfer_dirs = 0;
int ignore_perishable = 0;
int am_generator = 0;
int am_server = 0;
int delete_excluded = 0;
int delete_mode = 0;
int eol_nulls = 0;
int io_error = 0;
int local_server = 0;
int cvs_exclude = 0;
int prune_empty_dirs = 0;
int copy_links = 0;
int copy_unsafe_links = 0;
int dry_run = 0;
int read_only = 0;
int list_only = 0;
int am_root = 0;
int am_sender = 0;
int protocol_version = 30;

/* Indexed by INFO_ and DEBUG_ enum constants -- oversized on purpose so
 * any enumerator index rsync.h defines stays in bounds. */
short info_levels[64] = {0};
short debug_levels[64] = {0};

/* --- logging / diagnostics --- */
void rprintf(UNUSED(enum logcode code), const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

void rsyserr(UNUSED(enum logcode code), int errcode, const char *format, ...) {
    va_list ap;
    fputs(RSYNC_NAME ": ", stderr);
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, ": %s (%d)\n", strerror(errcode), errcode);
}

void _exit_cleanup(int code, const char *file, int line) {
    (void)code; (void)file; (void)line;
    /* No message on the hot path -- rprintf() already logged the reason
     * before this ran, and printing here on every rejected input would
     * dominate fuzzing time. */
    longjmp(fuzz_exit_jmp, 1);
}

const char *who_am_i(void) {
    return "fuzzer";
}

/* --- misc functions exclude.c references but this target doesn't hit --- */
int csum_len_for_type(int cst, int flg) {
    return cst || !flg ? 16 : 1;
}

int canonical_checksum(int cst) {
    (void)cst;
    return 0;
}

const char *default_cvsignore(void) {
    return "";
}

/* io.c protocol read/write stubs -- only reachable via send_filter_list()/
 * recv_filter_list(), which this fuzz target doesn't call. */
int32 read_int(int f) {
    (void)f;
    return 0;
}

void read_sbuf(int f, char *buf, size_t len) {
    (void)f;
    if (len)
        buf[0] = '\0';
}

void write_int(int f, int32 x) {
    (void)f; (void)x;
}

void write_buf(int f, const char *buf, size_t len) {
    (void)f; (void)buf; (void)len;
}

void write_byte(int f, uchar c) {
    (void)f; (void)c;
}

/* loadparm.c (daemon config) stubs -- filter parsing never queries a
 * daemon module's config in this fuzz target (module_id stays -1). */
BOOL lp_use_chroot(int mod) {
    (void)mod;
    return 0;
}

BOOL lp_insecure_links(int mod) {
    (void)mod;
    return 0;
}

char *lp_name(int mod) {
    (void)mod;
    return NULL;
}

int copy_xattrs(const char *source, int source_fd, const char *dest, int dest_fd) {
    (void)source; (void)source_fd; (void)dest; (void)dest_fd;
    return -1;
}
