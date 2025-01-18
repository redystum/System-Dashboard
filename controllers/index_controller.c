#include <stdio.h>
#include <stdlib.h>

#include "../html_parser.h"
#include "../utils.h"
#include "commands.h"
#include "index_controller.h"

parser_args_list_t returnService(void* args);

char* index_controller_init(char* file_path)
{

    ut_file_by_line_t* services_file = NULL;
    ut_dynamic_array_t services;
    ut_array_init(&services, sizeof(char*));

    if ((services_file = ut_file_by_line_open("data/services.data.txt")) == NULL) {
        WARNING("Failed to read file %s (look at the example and create a new one)", "data/services.data.txt");
        INFO("Continuing without services data");
    } else {
        INFO("Read %d bytes from %s", services_file->buffer_size, "data/services.data.txt");

        char* line;
        while ((line = ut_file_by_line_next(services_file)) != NULL) {
            ut_array_push(&services, line);
        }

        ut_file_by_line_close(services_file);
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
    parser_args_t p_args[] = {
        { .key = "cpu", .fun_cal = getCpuUsed, .fun_args = NULL },
        { .key = "ram_total", .fun_cal = getRamTotal, .fun_args = NULL },
        { .key = "ram", .fun_cal = getRamUsed, .fun_args = NULL },
        { .key = "disk", .fun_cal = getDiskUsed, .fun_args = NULL },
        { .key = "disk_total", .fun_cal = getDiskTotal, .fun_args = NULL },
        { .key = "disk_percentage", .fun_cal = getDiskPercentage, .fun_args = NULL },
        { .key = "temperature", .fun_cal = getTemperature, .fun_args = NULL },
        { .key = "services", .fun_cal = returnService, .fun_args = &services }
    };
#pragma GCC diagnostic pop

    parser_args_list_t p_args_list = { .args = p_args, .size = sizeof(p_args) / sizeof(p_args[0]) };

    return html_parse(file_path, p_args_list);
}

parser_args_list_t returnService(void* args)
{
    char* service = (char*)args;

    DEBUG("Service: %s", service);

    parser_args_t* p_args = malloc(4 * sizeof(parser_args_t));
    p_args[0] = (parser_args_t) { .key = "name", .fun_cal = NULL, .fun_args = "test.service" };
    p_args[1] = (parser_args_t) { .key = "cpu", .fun_cal = NULL, .fun_args = "2" };
    p_args[2] = (parser_args_t) { .key = "ram", .fun_cal = NULL, .fun_args = "150M" };
    p_args[3] = (parser_args_t) { .key = "started", .fun_cal = NULL, .fun_args = "Sat 2025-01-18 17:15:01 WET" };

    parser_args_list_t p_args_list = { .args = p_args, .size = 4 };
    return p_args_list;
}