#ifndef HTML_PARSER_H
#define HTML_PARSER_H

#include <stdlib.h>

typedef struct {
    char* key;
    void* (*fun_cal)();
    void* fun_args;
} parser_args_t;

typedef struct {
    parser_args_t* args;
    size_t size;
} parser_args_list_t;

char* html_parse(const char* file_name, parser_args_list_t args);
char* parse(char* html, parser_args_list_t args);

#endif