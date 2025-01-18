#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "html_parser.h"
#include "utils.h"

char* each(char* html, void* args);

char* html_parse(const char* file_name, parser_args_list_t args)
{
    char* html;

    int len = 0;
    if ((len = ut_read_file(file_name, &html)) < 0) {
        WARNING("Failed to read file %s", file_name);
        return NULL;
    }

    INFO("Read %d bytes from %s", len, file_name);

    char* new_html = parse(html, args);
    return new_html;
}

char* parse(char* html, parser_args_list_t args)
{
    ut_string_slice_t full_html_slice = { .str = html, .len = 0 };

    char* final_html = strdup("");
    if (final_html == NULL) {
        WARNING("Failed to allocate memory for final HTML");
        return NULL;
    }

    for (size_t i = 0; i < strlen(html); i++) {
        if (html[i] == '\0') {
            break;
        }

        if (html[i] == '{' && html[i + 1] != '\0' && html[i + 1] == '{') {
            DEBUG("Found opening {{");

            size_t j = 2;
            while (isspace(html[i + j])) {
                j++;
            }

            ut_string_slice_t slice = { .str = html + i + j, .len = 0 };
            while (html[i + j] != '\0' && html[i + j + 1] != '\0' && (html[i + j] != '}' && html[i + j + 1] != '}')) {
                slice.len += 1;
                j++;
            }

            while (isspace(html[i + j - 1])) {
                slice.len -= 1;
                j--;
            }
            char* val = NULL;
            ut_string_slice_original(&slice, &val);

            if (val[0] == '#') {
                ut_string_slice_t function_key_slice = { .str = val, .len = 0 };
                while (val[function_key_slice.len] != '\0' && !isspace(val[function_key_slice.len])) {
                    function_key_slice.len += 1;
                }
                char* function_key = NULL;
                ut_string_slice_original(&function_key_slice, &function_key);

                size_t start = function_key_slice.len;
                while (isspace(val[start])) {
                    start++;
                }

                ut_string_slice_t key_slice = { .str = val + start, .len = 0 };
                while (val[start + key_slice.len] != '\0' && !isspace(val[start + key_slice.len])) {
                    key_slice.len += 1;
                }

                char* key = NULL;
                ut_string_slice_original(&key_slice, &key);
                if (function_key != NULL && function_key[0] == '#') {
                    function_key += 1; // Remove '#'
                }

                DEBUG("Found function key: %s with key: %s", function_key, key);

                for (size_t j = 0; j < args.size; j++) {
                    parser_args_t arg = args.args[j];
                    if (strcmp(arg.key, key) == 0) {
                        if (strcmp(function_key, "each") == 0) {
                            DEBUG("Found each key");
                            char* res = each(val, arg.fun_args);
                            if (res == NULL) {
                                WARNING("Failed to parse each key");
                                break;
                            }

                            char* html_before = NULL;
                            ut_string_slice_original(&full_html_slice, &html_before);

                            char* temp_html = malloc((strlen(html_before) + strlen(res) + 1) * sizeof(char));
                            if (temp_html == NULL) {
                                free(final_html);
                                free(res);
                                free(val);
                                WARNING("Failed to allocate memory");
                                break;
                            }

                            strcpy(temp_html, html_before);
                            strcat(temp_html, res);

                            free(final_html);
                            final_html = temp_html;
                            free(val);
                            val = NULL;
                            free(res);
                            res = NULL;
                            break;
                        }
                    }
                }
            } else {
                DEBUG("Found key: %s", val);

                for (size_t j = 0; j < args.size; j++) {
                    parser_args_t arg = args.args[j];
                    if (strcmp(arg.key, val) == 0) {
                        char* res = NULL;
                        if (arg.fun_args != NULL) {
                            res = arg.fun_cal(arg.fun_args);
                        } else {
                            res = arg.fun_cal();
                        }
                        DEBUG("Replacing key with value: %s.", res);
                        if (res) {
                            strncpy(html + i, res, strlen(res));
                            i += strlen(res);
                        }
                        free(val);
                        break;
                    }
                }

                size_t k = i;
                while (html[k] != '\0' && !(html[k] == '}' && html[k + 1] == '}')) {
                    k++;
                }
                k += 2; // '}}'

                memmove(html + i, html + k, strlen(html + k) + 1);
            }
        }

        full_html_slice.len = i;
    }

    // DEBUG("Final HTML: %s", final_html);

    html = strdup(final_html);
    return html;
}

char* each(char* html, void* args)
{
    ut_dynamic_array_t* item = (ut_dynamic_array_t*)args;

    size_t final_html_size = strlen(html) * item->len + 1;
    char* final_html = malloc(final_html_size);
    if (final_html == NULL) {
        WARNING("Failed to allocate memory for final HTML");
        return NULL;
    }
    final_html[0] = '\0';

    for (size_t i = 0; i < item->len; i++) {
        strcat(final_html, html);
    }

    return final_html;
}
