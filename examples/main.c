#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "time.h"

#include "Block.h"

#include "apr_general.h"
#include "apr_pools.h"
#include "apr_strings.h"

#include "custache.h"

int main(void) {
    srand(time(NULL));

    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool, NULL);
    
    char *template = "{{#people}}{{#newline}}Hello, {{name}}.  You are {{age}} years old.  Your lucky number is {{lucky_number}}.{{/newline}}{{/people}}";
    const char *err;
    custache_t tpl = custache_compile(template, &err);

    #define num_people 3
    context_handler_b people[num_people] = {
        ^(const char *tag_key) {
            mustache_tag_t tag = { .type = MUSTACHE_TYPE_NONE };
            if (strcmp(tag_key, "name") == 0) {
                tag.type = MUSTACHE_TYPE_STRING;
                tag.as_string = "Derek";
            } else if (strcmp(tag_key, "age") == 0) {
                tag.type = MUSTACHE_TYPE_LONG;
                tag.as_long = 20;
            }
            return tag;
        }, ^(const char *tag_key) {
            mustache_tag_t tag = { .type = MUSTACHE_TYPE_NONE };
            if (strcmp(tag_key, "name") == 0) {
                tag.type = MUSTACHE_TYPE_STRING;
                tag.as_string = "Bertrand";
            } else if (strcmp(tag_key, "age") == 0) {
                tag.type = MUSTACHE_TYPE_LONG;
                tag.as_long = 50;
            }
            return tag;
        }, ^(const char *tag_key) {
            mustache_tag_t tag = { .type = MUSTACHE_TYPE_NONE };
            if (strcmp(tag_key, "name") == 0) {
                tag.type = MUSTACHE_TYPE_STRING;
                tag.as_string = "Grandpa";
            } else if (strcmp(tag_key, "age") == 0) {
                tag.type = MUSTACHE_TYPE_LONG;
                tag.as_long = 80;
            }
            return tag;
        }
    };

    mustache_tag_t *people_tags = apr_palloc(pool, num_people * sizeof(mustache_tag_t));
    for (size_t i = 0; i < num_people; i++) {
        people_tags[i].type = MUSTACHE_TYPE_CONTEXT;
        people_tags[i].as_context = people[i];
    }

    puts(custache_render(tpl, ^(const char *tag_key) {
        mustache_tag_t tag = { .type = MUSTACHE_TYPE_NONE };
        if (strcmp(tag_key, "people") == 0) {
            tag.type = MUSTACHE_TYPE_ARR;
            tag.arr_size = num_people;
            tag.as_arr = people_tags;
        }
        if (strcmp(tag_key, "newline") == 0) {
            tag.type = MUSTACHE_TYPE_DECORATOR;
            tag.as_decorator = Block_copy(^(const char *text, mustache_render_b render) {
                return apr_psprintf(pool, "%s\n", render(text));
            });
        }
        if (strcmp(tag_key, "lucky_number") == 0) {
            tag.type = MUSTACHE_TYPE_CALLABLE;
            tag.as_callable = Block_copy(^() {
                return apr_psprintf(pool, "%d", rand() % 100);
            });
        }
        return tag;
    }, &err));

    apr_pool_destroy(pool);
    apr_terminate();

    custache_release(tpl);
}

