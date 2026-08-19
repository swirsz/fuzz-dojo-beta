/*
 * fuzz_parse_rule.c
 *
 * libFuzzer driver for rsync's filter/exclude rule parser in exclude.c.
 *
 * There is no standalone parse_rule() in current rsync -- the tokenizer is
 * parse_rule_tok() (static), called in a loop by the public entry point:
 *
 *   void parse_filter_str(filter_rule_list *listp, const char *rulestr,
 *                          const filter_rule *template, int xflags);
 *
 * That's the fuzz target here. It's the natural boundary because it takes
 * a plain rule string -- the contents of --filter, --exclude-from, or a
 * per-directory .rsync-filter merge file -- and turns it into filter_rule
 * structs, exercising the full keyword grammar (clear/dir-merge/exclude/
 * hide/include/merge/protect/risk/show), the single-character
 * !/:/./+/-/H/P/R/S prefix forms, wildcard flag detection, and
 * add_rule()'s pattern-buffer construction on every call. Unlike
 * recv_file_entry() (see rsync_flist_fuzzer.c), parse_filter_str() is
 * public (non-static), so no source patching is needed to call it.
 *
 * Build notes (for build.sh): this harness follows the same whole-tree-
 * build, whole-object-list-link strategy as rsync_flist_fuzzer.c, and for
 * the same reason -- exclude.c's call graph reaches rprintf()/rsyserr()
 * (log.c), lp_use_chroot()/lp_name() (loadparm.c), copy_xattrs() (util1.c/
 * xattrs.c), default_cvsignore() (usage.c), and the protocol read/write
 * helpers (io.c) among others, and hand-picking a subset of .o files to
 * avoid pulling all that in is brittle -- it works until upstream adds a
 * call path through a file you didn't list. An earlier version of this
 * harness used a hand-written fuzz_stub.c providing ~20 stand-in globals
 * and functions instead of linking the real ones; that approach also
 * accidentally pulled in lib/sysacls.o (which needs -lacl, not present
 * in the OSS-Fuzz build image) for functionality nothing in this target's
 * call graph even reaches. Linking the whole tree with --disable-acl-
 * support at configure time (see build.sh) avoids both problems: the real
 * globals are simply defined by the real .o files, and ACL support is
 * compiled out upstream instead of needing to be avoided at link time.
 *
 * Like rsync_flist_fuzzer.c, exclude.c's parse path can call
 * _exit_cleanup() -> exit() on fatal errors (e.g. a `merge` rule naming
 * an unreadable file) -- fatal to the whole fuzzer process if left alone,
 * so build.sh links this with the same -Wl,--wrap=_exit_cleanup used for
 * the flist harness, redirecting to __wrap__exit_cleanup() below.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "rsync.h"

#ifdef FUZZ_DOJO_TRACE
#include "fuzz_dojo_trace.h"
#endif

extern int protocol_version;

/* See the block comment at the top of this file, and __wrap__exit_cleanup()
 * in rsync_flist_fuzzer.c for the full rationale -- this is the same
 * --wrap=_exit_cleanup interception pattern, just with its own jmp_buf
 * since this is a separate fuzz target binary.
 *
 * Not thread-safe: assumes libFuzzer's default single-threaded mode. */
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
    /* parse_filter_str() itself doesn't consult protocol_version, but
     * some of what it transitively links against (io.c's write_int() et
     * al., reached only on the send_rules()/recv_filter_list() paths we
     * don't call) assumes it's been set to something sane rather than
     * its static-zero default. Cheap to set, so just do it. */
    protocol_version = PROTOCOL_VERSION;
    initialized = 1;
}

/* Both need to be reachable from the longjmp landing spot below, since
 * __wrap__exit_cleanup() can fire mid-parse -- after rulestr was
 * allocated and/or after some rules were already linked into
 * g_listp.head. Static (not stack locals declared after setjmp()) so the
 * cleanup path can always reach and free whatever got allocated before
 * the jump, regardless of exactly where in parse_filter_str() the fatal
 * error happened. */
static char *g_rulestr;
static filter_rule_list g_listp;

static void free_rule_list(filter_rule_list *listp) {
    filter_rule *ent = listp->head;
    while (ent) {
        filter_rule *next = ent->next;
        if (ent->rflags & FILTRULE_PERDIR_MERGE) {
            free(ent->u.mergelist->debug_type);
            free(ent->u.mergelist);
        }
        free(ent->pattern);
        free(ent);
        ent = next;
    }
    memset(listp, 0, sizeof *listp);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    one_time_init();

    if (size == 0 || size > 65536) {
        return 0;
    }

    if (setjmp(fuzz_exit_jmp) != 0) {
        /* Landed here via __wrap__exit_cleanup()'s longjmp: this input
         * hit rsync's fatal-error path partway through parsing. Free
         * whatever had already been allocated so ASan/LSan stay quiet. */
        free_rule_list(&g_listp);
        free(g_rulestr);
        g_rulestr = NULL;
        return 0;
    }

    /* parse_filter_str() wants a NUL-terminated C string. */
    g_rulestr = (char *)malloc(size + 1);
    if (!g_rulestr) {
        return 0;
    }
    memcpy(g_rulestr, data, size);
    g_rulestr[size] = '\0';

    /* Reject embedded NULs: they'd just truncate the string early and
     * waste fuzzing cycles on a shorter effective input than reported. */
    if (strlen(g_rulestr) != size) {
        free(g_rulestr);
        g_rulestr = NULL;
        return 0;
    }

    memset(&g_listp, 0, sizeof g_listp);

    /* FILTRULE_NO_PREFIXES: plain +/- syntax same as --include/--exclude.
     * Drop it (rflags = 0) to exercise the full keyword-prefix grammar
     * plus the single-char prefix forms in parse_rule_tok(). */
    const filter_rule *template = rule_template(0);

    parse_filter_str(&g_listp, g_rulestr, template, 0);

#ifdef FUZZ_DOJO_TRACE
    FUZZ_DOJO_TRACE_ARG_PTR("parse_filter_str_result", g_listp.head);
#endif

    free_rule_list(&g_listp);
    free(g_rulestr);
    g_rulestr = NULL;
    return 0;
}
