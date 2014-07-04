#pragma once

#include <stddef.h>

typedef struct mustache_tag mustache_tag_t;

typedef mustache_tag_t (^mustache_tag_b)(const char *text);

typedef mustache_tag_t (^context_handler_b)(const char *tag);

// A compiled template that can be called with a context and returns a rendered page
typedef char *(^cweb_template_b)(context_handler_b);

// Compile a template
extern cweb_template_b cweb_template_compile(char *tpl);

typedef enum {
    MUSTACHE_TYPE_LONG,
    MUSTACHE_TYPE_DOUBLE,
    MUSTACHE_TYPE_STRING,
    MUSTACHE_TYPE_CONTEXT,

    MUSTACHE_TYPE_LONG_ARR,
    MUSTACHE_TYPE_DOUBLE_ARR,
    MUSTACHE_TYPE_STRING_ARR,
    MUSTACHE_TYPE_CONTEXT_ARR,

    MUSTACHE_TYPE_FUNC,

    MUSTACHE_TYPE_NONE
} mustache_tag_type_e;

struct mustache_tag {
    mustache_tag_type_e type;
    size_t arr_size;
    union {
        long as_long;
        double as_double;
        const char *as_string;
        context_handler_b as_context;

        long *as_longs;
        double *as_doubles;
        const char **as_strings;
        context_handler_b *as_contexts;

        mustache_tag_b as_func;
    };
};

