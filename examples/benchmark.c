#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "custache.h"

int main(void) {
    char *template = "{{#people}}Hello, {{name}}.  You are {{age}} years old.\n{{/people}}";
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

    mustache_tag_t *people_tags = malloc(num_people * sizeof(mustache_tag_t));
    for (size_t i = 0; i < num_people; i++) {
        people_tags[i].type = MUSTACHE_TYPE_CONTEXT;
        people_tags[i].as_context = people[i];
    }

    size_t num_iters = 1000000;
    for (size_t i = 0; i < num_iters; i++) {
        const char *err;
        custache_render(tpl, ^(const char *tag_key) {
            mustache_tag_t tag = { .type = MUSTACHE_TYPE_NONE };
            if (strcmp(tag_key, "people") == 0) {
                tag.type = MUSTACHE_TYPE_ARR;
                tag.arr_size = num_people;
                tag.as_arr = people_tags;
            }
            return tag;
        }, &err);
    }
    free(people_tags);
    custache_release(tpl);
}


