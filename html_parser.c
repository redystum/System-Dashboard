#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "html_parser.h"
#include "utils.h"

char* html_parse(const char* file_name, parser_args* args)
{
    char* html;

    int len = 0;
    if ((len = ut_read_file(file_name, &html)) < 0) {
        return NULL;
    }

    INFO("Read %d bytes from %s", len, file_name);

    parse(html, args);
    return html;
}

int parse(char* html, parser_args* args)
{

    for (size_t i = 0; i < strlen(html); i++) {

        if (html[i] == '\0') {
            break;
        }

        if (html[i] == '{' && html[i + 1] != '\0' && html[i + 1] == '{') {
            size_t j = 2;
            while (isspace(html[i + j])) {
                j++;
            }

            ut_string_slice_t slice = { .str = html + i + j, .len = 0 };
            while (html[i + j] != '\0' && html[i + j + 1] != '\0' && (html[i + j] != '}' && html[i + j + 1] != '}')) {
                DEBUG("char: %c", html[i+j]);
                slice.len += 1;
                j++;
            }
            char* val = NULL;
            ut_string_slice_original(&slice, &val);
            DEBUG("\nval: %s.\n", val);

            if (strcmp(val, "cpu") == 0) {
                INFO("CPU kay parsed!");
            } else if (strcmp(val, "ram") == 0) {
                INFO("RAM kay parsed!");
            }

            i += j;
        }
    }

    return 0;
}