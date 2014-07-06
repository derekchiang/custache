#include <stdarg.h>
#include <stdbool.h>
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
    custache_b current_template;
    const char *err;
} custache_sm_t;

// A naive algorithm that walks through src and returns the position of the first occurrence of any
// of the given patterns
static const char *find_one_of_these(const char *src, const char **which_one, ...) {
    va_list patterns;
    size_t pos = 0;
    while (src[pos] != '\0') {
        va_start(patterns, which_one);
        while (true) {
            const char *pat = va_arg(patterns, const char *);
            if (!pat) break;
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
    const char *text_end = find_one_of_these(csm.remaining, &pat, "{{#", "{{", NULL);

    // The order in which we compare these strings is important;
    // for instance, if it were {{# and we compared it was {{ first, we'd think it's just {{
    if (!text_end) {
        text_end = csm.remaining + strlen(csm.remaining);
        csm.state = DONE;
    } else if (strcmp(pat, "{{#") == 0) {
        csm.state = EXPECTING_SECTION;
    } else if (strcmp(pat, "{{") == 0) {
        csm.state = EXPECTING_VAR;
    }

    custache_b prev_tpl = csm.current_template;
    csm.current_template = Block_copy(^(context_handler_b context) {
        char *ret = bu_format("%s%s", prev_tpl(context),
                              apr_pstrmemdup(bu_current_pool(),
                                             csm.remaining, text_end - csm.remaining));
        return ret;
    });

    csm.remaining = text_end;
    return csm;
}

static const char *extract_tag_key(const char *src, const char **tag_key) {
    const char *tag_start_pos = strstr(src, "{{") + 2;
    const char *tag_end_pos = strstr(src, "}}");
    *tag_key = apr_pstrmemdup(bu_current_pool(), tag_start_pos, tag_end_pos - tag_start_pos);
    return tag_end_pos + 2;
}

static custache_sm_t transit_from_expecting_var(custache_sm_t csm) {
    const char *tag_key;
    csm.remaining = extract_tag_key(csm.remaining, &tag_key);

    csm.state = EXPECTING_TEXT;

    custache_b prev_tpl = csm.current_template;
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
        return ret;
    });
    
    if (csm.remaining[0] == '\0') csm.state = DONE;
    return csm;
}

static const char *extract_tag_key_and_section(const char *src,
                                               const char **tag_key,
                                               const char **section) {
    const char *pos;
    const char *open_tag;
    pos = extract_tag_key(src, &open_tag);
    const char *section_start = pos;

    assert(open_tag[0] == '#');
    open_tag = open_tag + 1;
    
    const char *end_tag;
    while (true) {
        pos = extract_tag_key(pos, &end_tag);
        if (end_tag[0] == '/' || strcmp(open_tag, end_tag + 1) == 0) {
            *tag_key = open_tag;
            const char *section_end = pos - strlen(bu_format("{{/%s}}", open_tag));
            *section = apr_pstrmemdup(bu_current_pool(), section_start, section_end - section_start);
            return pos;
        }
    }
}

static context_handler_b combine_contexts(context_handler_b ctx1, context_handler_b ctx2) {
    return Block_copy(^(const char *tag_key) {
        mustache_tag_t tag = ctx1(tag_key);
        if (tag.type == MUSTACHE_TYPE_NONE) {
            return ctx2(tag_key);
        } else {
            return tag;
        }
    });
}

static bool tag_is_truthy(mustache_tag_t tag) {
    if ( (tag.type == MUSTACHE_TYPE_LONG && tag.as_long == 0)
         || (tag.type == MUSTACHE_TYPE_DOUBLE && tag.as_double == 0)
         || (tag.type == MUSTACHE_TYPE_STRING && strlen(tag.as_string) == 0)
         || (tag.type == MUSTACHE_TYPE_ARR && tag.arr_size == 0)) {
        return false;
    } else {
        return true;
    }
}

static const char *render_section(context_handler_b context,
                                  custache_b section_tpl,
                                  mustache_tag_t tag) {
    const char *ret;
    switch (tag.type) {
    case MUSTACHE_TYPE_LONG:
    case MUSTACHE_TYPE_DOUBLE:
    case MUSTACHE_TYPE_STRING:
        if (tag_is_truthy(tag)) return section_tpl(context);
        else return "";
    case MUSTACHE_TYPE_CONTEXT:;
        context_handler_b child_context = tag.as_context;
        context_handler_b combined_context = combine_contexts(child_context, context);
        ret = section_tpl(combined_context);
        return ret;
    case MUSTACHE_TYPE_ARR:;
        ret = "";
        for (size_t i = 0; i < tag.arr_size; i++) {
            ret = apr_pstrcat(bu_current_pool(), ret,
                              render_section(context, section_tpl, tag.as_arr[i]), NULL);
        } 
        return ret;
    }
    return "";
}

static custache_sm_t transit_from_expecting_section(custache_sm_t csm) {
    const char *tag_key;
    const char *section;
    csm.remaining = extract_tag_key_and_section(csm.remaining, &tag_key, &section);

    custache_b prev_tpl = csm.current_template;
    const char *err;
    custache_b section_tpl = custache_compile(section, &err);
    csm.current_template = Block_copy(^(context_handler_b context) {
        char *ret = apr_pstrcat(bu_current_pool(),
                    prev_tpl(context),
                    render_section(context, section_tpl, context(tag_key)), NULL);
        return ret;
    });
    
    csm.state = EXPECTING_TEXT;

    return csm;
}

custache_b custache_compile(const char *tpl, const char **err) {
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
        case DONE:
            break;
        }
    }


    return Block_copy(csm.current_template);
}

void _custache_run_tests(void) {
    const char *src = "Hello, {{name}}";
    const char *which;
    const char *pos = find_one_of_these(src, &which, "<<", "{{", NULL);
    assert(strcmp(which, "{{") == 0);
    assert(pos - src == 7);
}

