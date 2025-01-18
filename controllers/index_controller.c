#include <stdio.h>

#include "../html_parser.h"
#include "../utils.h"
#include "commands.h"
#include "index_controller.h"

ut_dynamic_array_t returnServices(void* args);

char* index_controller_init(char* file_path)
{

    ut_file_by_line_t* services_file = NULL;
    ut_dynamic_array_t services = { .data = NULL, .len = 0, .cap = 0, .size = sizeof(char*) };

    if ((services_file = ut_file_by_line_open("data/services.data.txt")) == NULL) {
        WARNING("Failed to read file %s (look at the example and create a new one)", "data/services.data.txt");
        INFO("Continuing without services data");
    } else {
        INFO("Read %d bytes from %s", services_file->buffer_size, "data/services.data.txt");

        char* line;
        while ((line = ut_file_by_line_next(services_file)) != NULL) {
            ut_array_push(&services, &line);
        }

        ut_file_by_line_close(services_file);
    }

#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
    parser_args_t p_args[] = {
        { .key = "cpu", .fun_cal = getCpuUsed, .fun_args = NULL },
        { .key = "ram_total", .fun_cal = getRamTotal, .fun_args = NULL },
        { .key = "ram", .fun_cal = getRamUsed, .fun_args = NULL },
        { .key = "disk", .fun_cal = getDiskUsed, .fun_args = NULL },
        { .key = "disk_total", .fun_cal = getDiskTotal, .fun_args = NULL },
        { .key = "disk_percentage", .fun_cal = getDiskPercentage, .fun_args = NULL },
        { .key = "temperature", .fun_cal = getTemperature, .fun_args = NULL },
        { .key = "services", .fun_cal = NULL, .fun_args = &services }
    };

    parser_args_list_t p_args_list = { .args = p_args, .size = sizeof(p_args) / sizeof(p_args[0]) };

    return html_parse(file_path, p_args_list);
}

ut_dynamic_array_t returnServices(void* args)
{
    ut_dynamic_array_t* services = (ut_dynamic_array_t*)args;
#ifdef DEBUG_ENABLED
    for (unsigned int i = 0; i < services->len; i++) {
        DEBUG("Service: %s", *(char**)ut_array_get(services, i));
    }
#endif
    return *services;
}