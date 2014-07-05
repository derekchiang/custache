#include "string.h"
#include "stdio.h"

#include "Block.h"

#include "apr_general.h"

#include "block_utils.h"

#include "custache.h"

int main(void) {
    apr_initialize();
    
    bu_with_pool(^{
        _custache_run_tests();

        char *template = "{{#people}}Hello, {{name}}.  You are {{age}} years old.\n{{/people}}";
        const char *err;
        custache_b tpl = custache_compile(template, &err);

        #define num_people 2
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
            }
        };

        mustache_tag_t *people_tags = bu_alloc(num_people * sizeof(mustache_tag_t));
        for (size_t i = 0; i < num_people; i++) {
            people_tags[i].type = MUSTACHE_TYPE_CONTEXT;
            people_tags[i].as_context = people[i];
        }

        puts(tpl(^(const char *tag_key) {
            mustache_tag_t tag = { .type = MUSTACHE_TYPE_NONE };
            if (strcmp(tag_key, "people") == 0) {
                tag.type = MUSTACHE_TYPE_ARR;
                tag.arr_size = num_people;
                tag.as_arr = people_tags;
            }
            return tag;
        }));
    });

    apr_terminate();
}

