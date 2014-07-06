#pragma once

#include <stddef.h>

typedef struct mustache_tag mustache_tag_t;

typedef char *(^mustache_render_b)(const char *text);

typedef char *(^mustache_func_b)(const char *text, mustache_render_b);

typedef mustache_tag_t (^context_handler_b)(const char *tag);

// A compiled template that can be called with a context and returns a rendered page
typedef char *(^custache_b)(context_handler_b);

// Compile a template
extern custache_b custache_compile(const char *tpl, const char **err);

extern void _custache_run_tests(void);

typedef enum {
    MUSTACHE_TYPE_LONG,
    MUSTACHE_TYPE_DOUBLE,
    MUSTACHE_TYPE_STRING,
    MUSTACHE_TYPE_CONTEXT,

    MUSTACHE_TYPE_ARR,
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

        mustache_tag_t *as_arr;
        mustache_func_b as_func;
    };
};

