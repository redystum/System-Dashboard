#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
            char* line_copy = strdup(line);
            ut_array_push(&services, &line_copy);
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

    char* html = html_parse(file_path, p_args_list);

    for (size_t i = 0; i < services.len; i++) {
        free(*(char**)ut_array_get(&services, i));
    }

    ut_array_free(&services);

    return html;
}

parser_args_list_t returnService(void* args) {
    char* service = *(char**)args;
    DEBUG("Service: %s", service);

    char command[256];
    parser_args_t* p_args = malloc(4 * sizeof(parser_args_t));

    #ifdef RASPBERRYPI
        snprintf(command, sizeof(command), "systemctl show -p Description --value %s", service);
    #else
        snprintf(command, sizeof(command), "echo %s", service); // Simulated service name
    #endif
    char* name = runCommand(command);
    name[strcspn(name, "\n")] = '\0'; // Strip newline
    p_args[0] = (parser_args_t) { .key = "name", .fun_cal = NULL, .fun_args = name };

    #ifdef RASPBERRYPI
        snprintf(command, sizeof(command), "systemctl status %s | grep 'CPU:' | awk '{print $2}'", service);
    #else
        snprintf(command, sizeof(command), "echo 0.2"); // Simulated CPU usage
    #endif
    char* cpu = runCommand(command);
    cpu[strcspn(cpu, "\n")] = '\0';
    p_args[1] = (parser_args_t) { .key = "cpu", .fun_cal = NULL, .fun_args = cpu };

    #ifdef RASPBERRYPI
        snprintf(command, sizeof(command), "systemctl status %s | grep 'Memory:' | awk '{print $2, $3, $4}'", service);
    #else
        snprintf(command, sizeof(command), "echo 160.0K (peak: 1.1M)"); // Simulated RAM usage in KB
    #endif
    char* ram = runCommand(command);
    ram[strcspn(ram, "\n")] = '\0';
    p_args[2] = (parser_args_t) { .key = "ram", .fun_cal = NULL, .fun_args = ram };

    #ifdef RASPBERRYPI
        snprintf(command, sizeof(command), "systemctl status %s | grep 'Active:' | awk '{print $6, $7, $8, $9, $10}'", service);
    #else
        snprintf(command, sizeof(command), "echo 2021-01-01 00:00:00"); // Simulated start time
    #endif
    char* started = runCommand(command);
    started[strcspn(started, "\n")] = '\0';
    p_args[3] = (parser_args_t) { .key = "started", .fun_cal = NULL, .fun_args = started };

    parser_args_list_t p_args_list = { .args = p_args, .size = 4 };
    return p_args_list;
}
