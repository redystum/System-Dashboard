#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "html_parser.h"
#include "utils.h"

char* html_parse(const char* file_name, parser_args_list_t args)
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

int parse(char* html, parser_args_list_t args)
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
                slice.len += 1;
                j++;
            }
            int k = j;
            while (isspace(html[i + k - 1])) {
                slice.len -= 1;
                k--;
            }
            char* val = NULL;
            ut_string_slice_original(&slice, &val);

            for (size_t k = 0; k < args.size; k++) {
                if (strcmp(args.args[k].key, val) == 0) {
                    char* res = args.args[k].fun_cal();
                    if (res) {
                        strncpy(html + i, res, strlen(res));
                        i += strlen(res);
                    }
                    free(val);
                    break;
                }
            }

            while (html[i] != '\0' && !(html[i] == '}' && html[i + 1] == '}')) {
                html[i++] = ' ';
            }
            html[i++] = ' ';
            html[i++] = ' ';

            i += j;
        }
    }

    return 0;
}