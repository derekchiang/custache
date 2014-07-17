#pragma once

#include <stddef.h>

typedef struct mustache_tag mustache_tag_t;

typedef char *(^mustache_render_b)(const char *text);

typedef char *(^mustache_callable_b)();

typedef char *(^mustache_decorator_b)(const char *text, mustache_render_b);

typedef mustache_tag_t (^context_handler_b)(const char *tag); // A compiled template that can be called with a context and returns a rendered page

typedef char *(^custache_b)(context_handler_b);

typedef struct custache *custache_t;

// Compile a template
extern custache_t custache_compile(const char *tpl, const char **err);

// Render a template with a context
extern char *custache_render(custache_t, context_handler_b, const char **err);

// Release all internal memory used by the template.  The template cannot be used after this call.
extern void custache_release(custache_t);

extern void _custache_run_tests(void);

typedef enum {
    MUSTACHE_TYPE_LONG,
    MUSTACHE_TYPE_DOUBLE,
    MUSTACHE_TYPE_STRING,
    MUSTACHE_TYPE_CONTEXT,

    MUSTACHE_TYPE_ARR,
    MUSTACHE_TYPE_CALLABLE,
    MUSTACHE_TYPE_DECORATOR,
    MUSTACHE_TYPE_NONE
} mustache_tag_type_e;

typedef void (^custache_cleanup_b)(mustache_tag_t tag);

struct mustache_tag {
    mustache_tag_type_e type;
    size_t arr_size;
    union {
        long as_long;
        double as_double;
        const char *as_string;
        context_handler_b as_context;

        mustache_tag_t *as_arr;
        mustache_callable_b as_callable;
        mustache_decorator_b as_decorator;
    };
    custache_cleanup_b cleanup;  // this block will be called after the tag has been used
};

