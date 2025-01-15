#ifndef HTML_PARSER_H
#define HTML_PARSER_H

typedef struct {
    char* key;
    char* val;
} parser_args;

char *html_parse(const char *file_name, parser_args* args);
int parse(char *html, parser_args* args);

#endif