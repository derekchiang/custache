#include <stdarg.h>
#include <assert.h>

#include "Block.h"

#include "apr_strings.h"
#include "apr_tables.h"

#include "block_utils.h"

#include "custache.h"

typedef enum {
    EXPECTING_TEXT,
    EXPECTING_VAR,
    EXPECTING_SECTION,
    ERROR,
    DONE
} custache_sm_state_e;

typedef struct custache_sm {
    custache_sm_state_e state;
    const char *remaining;
    cweb_template_b current_template;
    const char *err;
} custache_sm_t;

// A naive algorithm that walks through src and returns the position of the first occurrence of any
// of the given patterns
static const char *find_one_of_these(const char *src, const char **which_one, int count, ...) {
    va_list patterns;
    size_t pos = 0;
    while (src[pos] != '\0') {
        va_start(patterns, count);
        for (int i = 0; i < count; i++) {
            const char *pat = va_arg(patterns, const char *);
            if (strncmp(src + pos, pat, strlen(pat)) == 0) {
                *which_one = pat;
                return src + pos;
            }
        }
        va_end(patterns);
        pos++;
    }
    return NULL;
}

static custache_sm_t transit_from_expecting_text(custache_sm_t csm) {
    const char *pat;
    const char *tag_start_pos = find_one_of_these(csm.remaining, &pat,  2, "{{#", "{{");

    // The order in which we compare these strings is important;
    // for instance, if it were {{# and we compared it was {{ first, we'd think it's just {{
    if (!tag_start_pos) {
        tag_start_pos = csm.remaining + strlen(csm.remaining);
        csm.state = DONE;
    } else if (strcmp(pat, "{{#") == 0) {
        csm.state = EXPECTING_SECTION;
    } else if (strcmp(pat, "{{") == 0) {
        csm.state = EXPECTING_VAR;
    }
    cweb_template_b prev_tpl = csm.current_template;
    csm.current_template = Block_copy(^(context_handler_b context) {
        char *ret = bu_format("%s%s", prev_tpl(context),
                              apr_pstrmemdup(bu_current_pool(),
                                             csm.remaining, tag_start_pos - csm.remaining));
        Block_release(prev_tpl);
        return ret;
    });

    csm.remaining = tag_start_pos;
    return csm;
}

static const char *extract_tag_key(const char *src, const char **end_pos) {
    const char *tag_start_pos = strstr(src, "{{") + 2;
    const char *tag_end_pos = strstr(src, "}}");
    *end_pos = tag_end_pos + 2;
    return apr_pstrmemdup(bu_current_pool(), tag_start_pos, tag_end_pos - tag_start_pos);
}

static custache_sm_t transit_from_expecting_var(custache_sm_t csm) {
    const char *tag_end_pos;
    const char *tag_key = extract_tag_key(csm.remaining, &tag_end_pos);

    csm.state = EXPECTING_TEXT;

    cweb_template_b prev_tpl = csm.current_template;
    csm.current_template = Block_copy(^(context_handler_b context) {
        mustache_tag_t tag = context(tag_key);
        char *str;
        switch (tag.type) {
        case MUSTACHE_TYPE_LONG:
            str = bu_format("%lu", tag.as_long);
            break;
        case MUSTACHE_TYPE_DOUBLE:
            str = bu_format("%f", tag.as_double);
            break;
        case MUSTACHE_TYPE_STRING:
            str = bu_format("%s", tag.as_string);  // we want to make a copy of the string
            break;
        default:
            return (char *) NULL;  // TODO: better error reporting
        }
        char *ret = bu_format("%s%s", prev_tpl(context), str);
        Block_release(prev_tpl);
        return ret;
    });
    
    csm.remaining = tag_end_pos;
    if (csm.remaining[0] == '\0') csm.state = DONE;
    return csm;
}

static custache_sm_t transit_from_expecting_section(custache_sm_t csm) {
    return csm;
}

cweb_template_b cweb_template_compile(const char *tpl, const char **err) {
    custache_sm_t csm = {
        .state = EXPECTING_TEXT,
        .remaining = tpl,
        .current_template = ^(context_handler_b context) {
            return "";
        },
        .err = NULL
    };

    size_t len = strlen(tpl);
    while (csm.state != DONE) {
        switch (csm.state) {
        case EXPECTING_TEXT:
            csm = transit_from_expecting_text(csm);
            break;
        case EXPECTING_VAR:
            csm = transit_from_expecting_var(csm);
            break;
        case EXPECTING_SECTION:
            csm = transit_from_expecting_section(csm);
            break;
        case ERROR:
            *err = csm.err;
            return NULL;
        }
    }


    return Block_copy(csm.current_template);
}

void _custache_run_tests(void) {
    const char *src = "Hello, {{name}}";
    const char *which;
    const char *pos = find_one_of_these(src, &which, 2, "<<", "{{");
    assert(strcmp(which, "{{") == 0);
    assert(pos - src == 7);
}

