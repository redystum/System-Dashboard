#ifndef HTML_PARSER_H
#define HTML_PARSER_H

typedef struct {
    char* key;
    char* (*fun_cal)(void);
} parser_args_t;

typedef struct {
    parser_args_t* args;
    size_t size;
} parser_args_list_t;

char* html_parse(const char* file_name, parser_args_list_t args);
int parse(char* html, parser_args_list_t args);

#endif