#ifndef SERVER_H
#define SERVER_H

#include "html_parser.h"

typedef struct {
    char* file;
    char* (*fun)(char* file_path);
} controller_t;

typedef struct {
    controller_t* controllers;
    size_t size;
} controller_list_t;

int server_init(int port, controller_list_t controllers);

#endif