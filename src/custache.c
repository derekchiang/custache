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
    const char *current_str = "";

    apr_array_header_t *tokens = apr_array_make(pool, 0, sizeof(token_t));
    
    const char *first = tpl;
    const char *last;
    while (first < (tpl + len)) {
        last = strstr(first, "{{");
        if (!last) {
            last = tpl + len;
        }
        token_t str_token = {
            .type = TOKEN_STRING,
            .as_string = apr_pstrmemdup(pool, first, last - first)
        };
        APR_ARRAY_PUSH(tokens, token_t) = str_token;

        if (last >= (tpl + len)) break;

        first = last + 2;
        last = strstr(first, "}}");
        token_t tag_token = {
            .type = TOKEN_TAG,
            .as_string = apr_pstrmemdup(pool, first, last - first)
        };
        first = last + 2;

        APR_ARRAY_PUSH(tokens, token_t) = tag_token;
    }

    return Block_copy(^(context_handler_b context) {
        const char *ret = "";

        for (size_t i = 0; i < tokens->nelts; i++) {
            token_t token = APR_ARRAY_IDX(tokens, i, token_t);
            if (token.type == TOKEN_STRING) {
                ret = apr_pstrcat(pool, ret, token.as_string, NULL);
            } else if (token.type == TOKEN_TAG) {
                mustache_tag_t tag = context(token.as_string);
                switch (tag.type) {
                case MUSTACHE_TYPE_STRING:
                    ret = apr_pstrcat(pool, ret, tag.as_string, NULL);
                    break;
                case MUSTACHE_TYPE_LONG:
                    ret = apr_pstrcat(pool, ret, bu_format("%ld", tag.as_long), NULL);
                    break;
                }
            }
        }

        return ret;
    });
} 

