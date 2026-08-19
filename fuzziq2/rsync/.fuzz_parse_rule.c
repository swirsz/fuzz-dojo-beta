/*
 * fuzz_parse_rule.c
 *
 * libFuzzer driver for rsync's filter/exclude rule parser in exclude.c.
 *
 * There is no standalone parse_rule() in current rsync -- the tokenizer is
 * parse_rule_tok() (static), called in a loop by the public entry point
 * parse_filter_str(filter_rule_list *listp, const char *rulestr,
 *                   const filter_rule *template, int xflags).
 * That's the natural fuzz boundary: it takes an attacker/user-controlled
 * rule string (e.g. the contents of a --filter, --exclude-from, or a
 * per-directory .rsync-filter merge file) and turns it into filter_rule
 * structs, exercising parse_rule_tok() + add_rule() on every call.
 */

#include "rsync.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

/* Defined in fuzz_stub.c; _exit_cleanup() longjmps here instead of calling
 * exit(), so a rule string that hits rsync's fatal-error path just aborts
 * this one iteration instead of killing the fuzzer process. */
extern jmp_buf fuzz_exit_jmp;

int LLVMFuzzerInitialize(int *argc, char ***argv) {
    (void)argc;
    (void)argv;
    return 0;
}

/* Both need to be reachable from the longjmp landing spot below, since
 * _exit_cleanup() can fire mid-parse -- after rulestr was allocated and/or
 * after some rules were already linked into listp.head. Static (not stack
 * locals declared after setjmp()) so the cleanup path can always reach
 * and free whatever got allocated before the jump, regardless of exactly
 * where in parse_filter_str() the fatal error happened. */
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
    if (size == 0 || size > 65536)
        return 0;

    if (setjmp(fuzz_exit_jmp) != 0) {
        /* Landed here via longjmp from _exit_cleanup(): this input hit
         * rsync's fatal-error path partway through parsing. Free whatever
         * had already been allocated so ASan/leak-sanitizer stays quiet. */
        free_rule_list(&g_listp);
        free(g_rulestr);
        g_rulestr = NULL;
        return 0;
    }

    /* parse_filter_str() wants a NUL-terminated C string. */
    g_rulestr = (char *)malloc(size + 1);
    if (!g_rulestr)
        return 0;
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
     * Drop it (rflags = 0) to exercise the full "clear/dir-merge/exclude/
     * hide/include/merge/protect/risk/show" keyword-prefix grammar plus
     * the single-char !/:/./+/-/H/P/R/S forms in parse_rule_tok(). */
    const filter_rule *template = rule_template(0);

    parse_filter_str(&g_listp, g_rulestr, template, 0);

    free_rule_list(&g_listp);
    free(g_rulestr);
    g_rulestr = NULL;
    return 0;
}