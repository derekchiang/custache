#include "Block.h"

#include "apr_strings.h"
#include "apr_tables.h"

#include "block_utils.h"

#include "custache.h"

typedef enum {
    EXPECTING_TEXT,
    EXPECTING_TAG,
} custache_sm_state_e;

typedef const char *(^string_b)(void);

typedef struct custache_sm {
    custache_sm_state_e state;
    const char *remaining;
    cweb_template_b current_template;
} custache_sm_t;

typedef enum {
    TOKEN_STRING,
    TOKEN_TAG
} token_type_e;

typedef struct token {
    token_type_e type;
    union {
        const char *as_string;
    };
} token_t;

cweb_template_b cweb_template_compile(char *tpl) {
    apr_pool_t *pool = bu_current_pool();

    size_t len = strlen(tpl);

    custache_sm_t csm = {
        .state = EXPECTING_TEXT,
        .remaining = tpl,
        .current_template = ^(context_handler_b context){
            return "";
        }
    };
    
    while (csm.remaining < (tpl + len)) {
        const char *pos;
        cweb_template_b prev_tpl = Block_copy(csm.current_template);

        switch (csm.state) {
        case EXPECTING_TEXT:
            pos = strstr(csm.remaining, "{{");
            if (!pos) pos = tpl + len;
            csm.current_template = ^(context_handler_b context) {
                const char *prev_text = prev_tpl(context);
                Block_release(prev_tpl);
                return bu_format("%s%s",
                                 prev_text,
                                 apr_pstrmemdup(pool, csm.remaining, pos - csm.remaining));
            };
            csm.remaining = pos + 2;
            csm.state = EXPECTING_TAG;
            break;
        case EXPECTING_TAG:
            pos = strstr(csm.remaining, "}}");
            if (!pos) pos = tpl + len;
            csm.current_template = ^(context_handler_b context) {
                const char *prev_text = prev_tpl(context);
                Block_release(prev_tpl);

                const char *tag_key = apr_pstrmemdup(pool, csm.remaining, pos - csm.remaining);
                mustache_tag_t tag = context(tag_key);
                switch (tag.type) {
                case MUSTACHE_TYPE_STRING:
                    return bu_format("%s%s", prev_text, tag.as_string);
                    break;
                case MUSTACHE_TYPE_LONG:
                    return bu_format("%s%ld", prev_text, tag.as_long);
                    break;
                }
                return csm.current_template(context);
            };
            csm.remaining = pos + 2;
            csm.state = EXPECTING_TEXT;
            break;
        }
    }

    return Block_copy(csm.current_template);
} 

