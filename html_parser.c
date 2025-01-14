#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "html_parser.h"
#include "utils.h"


char *html_parse(const char *file_name) {
    char *html;

    int len = 0;
    if ((len = ut_read_file(file_name, &html)) < 0) {
        return NULL;
    }

    INFO("Read %d bytes from %s", len, file_name);

    char *replaced_html = strdup(html);
    free(html);
    return replaced_html;
}