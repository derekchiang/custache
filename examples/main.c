#include "string.h"
#include "stdio.h"

#include "apr_general.h"

#include "block_utils.h"

#include "custache.h"

int main(void) {
    apr_initialize();
    
    bu_with_pool(^{
        _custache_run_tests();

        char *template = "Hello, {{name}}.{{#show_age}}  You are {{age}} years old.{{/show_age}}";
        const char *err;
        cweb_template_b tpl = cweb_template_compile(template, &err);

        puts(tpl(^(const char *tag_key) {
            mustache_tag_t tag = { .type = MUSTACHE_TYPE_NONE };
            if (strcmp(tag_key, "name") == 0) {
                tag.type = MUSTACHE_TYPE_STRING;
                tag.as_string = "Derek";
            } else if (strcmp(tag_key, "age") == 0) {
                tag.type = MUSTACHE_TYPE_LONG;
                tag.as_long = 20;
            } else if (strcmp(tag_key, "show_age") == 0) {
                tag.type = MUSTACHE_TYPE_LONG;
                tag.as_long = 1;
            }
            return tag;
        }));
    });

    apr_terminate();
}

