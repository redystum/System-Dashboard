#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../html_parser.h"
#include "../utils.h"
#include "commands.h"
#include "services_controller.h"

char* services_controller_init(char* file_path)
{

    char* services = NULL;
#ifdef RASPBERRYPI
    services = runCommandNoNewLine("systemctl list-units --type=service --all --no-legend --no-pager --output=json | jq -r '.[] | \"\\(.unit)|\\(.sub)|\\(.description)\"'");
#else
    services = strdup("sysdash.service|running|System Dashboard\nsysdash.service|exited|System Dashboard\nsysdash.service|running|System Dashboard");
#endif

    if (services == NULL) {
        WARNING("Failed to get services");
        return NULL;
    }

    DEBUG("Services: %s", services);

    int services_len = 0;

    /*
        the final thing:
        (parser_args_list){.args= (parser_args_t){ .fun=NULL, .args= (ut_din_arr){(parser_args_list){args=(parser_args){...}, size=x}} }, size=1}

        I could have done this in js on the client. Yes
        And it would be easier. I know
        I want to do it in js? NO
     */

    ut_dynamic_array_t services_array;
    ut_array_init(&services_array, sizeof(parser_args_list_t));

    char* service;
    char* rest = services;
    while ((service = strtok_r(rest, "\n", &rest))) {
        parser_args_list_t p_args_list;

        parser_args_t* p_args = malloc(3 * sizeof(parser_args_t));
        if (p_args == NULL) {
            WARNING("Failed to allocate memory for parser_args_t");
            ut_array_free(&services_array);
            free(services);
            return NULL;
        }

        char* token;
        char* service_rest = service;
        int service_position = 0;
        while ((token = strtok_r(service_rest, "|", &service_rest))) {
            p_args[service_position].fun_cal = NULL;
            if (service_position == 0) {
                p_args[service_position].key = "unit";
                p_args[service_position].fun_args = strdup(token);
            } else if (service_position == 1) {
                p_args[service_position].key = "status";
                p_args[service_position].fun_args = strcmp(token, "running") == 0 ? strdup("1") : strdup("0");
            } else if (service_position == 2) {
                p_args[service_position].key = "description";
                p_args[service_position].fun_args = strdup(token);
            }
            service_position++;
        }

        p_args_list = (parser_args_list_t) { .args = p_args, .size = 3 };
        ut_array_push(&services_array, &p_args_list);

        services_len++;
    }

    free(services);

    parser_args_t* p_args = malloc(1 * sizeof(parser_args_t));
    if (p_args == NULL) {
        WARNING("Failed to allocate memory for parser_args_t");
        for (size_t i = 0; i < services_array.len; i++) {
            parser_args_list_t* p_args_list = ut_array_get(&services_array, i);
            for (size_t j = 0; j < p_args_list->size; j++) {
                free(p_args_list->args[j].fun_args);
            }
            free(p_args_list->args);
        }
        ut_array_free(&services_array);
        return NULL;
    }

    p_args[0] = (parser_args_t) { .key = "services", .fun_cal = NULL, .fun_args = &services_array };
    parser_args_list_t p_args_list = { .args = p_args, .size = 1 };

    char* html = html_parse(file_path, p_args_list);

    for (size_t i = 0; i < services_array.len; i++) {
        parser_args_list_t* p_args_list = ut_array_get(&services_array, i);
        for (size_t j = 0; j < p_args_list->size; j++) {
            free(p_args_list->args[j].fun_args);
        }
        free(p_args_list->args);
    }

    ut_array_free(&services_array);
    free(p_args);

    return html;
}

char* add_to_relevant_server_list(char* service_name)
{
    ut_file_by_line_t* services_file = NULL;

    if ((services_file = ut_file_by_line_open("data/services.data.txt")) == NULL) {
        WARNING("Failed to read file %s: Not found ", "data/services.data.txt");
        INFO("Creating file %s", "data/services.data.txt");

        FILE* file = fopen("data/services.data.txt", "w");
        if (file == NULL) {
            WARNING("Failed to create file %s", "data/services.data.txt");
            return strdup("ERROR");
        }
        fclose(file);
    } else {
        INFO("Read %d bytes from %s", services_file->buffer_size, "data/services.data.txt");

        char* line;
        while ((line = ut_file_by_line_next(services_file)) != NULL) {
            if (line == NULL) {
                WARNING("Failed to read line from file");
                ut_file_by_line_close(services_file);
                return strdup("ERROR");
            }

            ut_trim(line);

            if (strcmp(line, service_name) == 0) {
                ut_file_by_line_close(services_file);
                DEBUG("Service %s already in list", service_name);
                return strdup("OK");
            }
        }

        ut_file_by_line_close(services_file);
    }

    FILE* file = fopen("data/services.data.txt", "a");
    if (file == NULL) {
        WARNING("Failed to open file %s", "data/services.data.txt");
        return strdup("ERROR");
    }

    fprintf(file, "\n%s", service_name);

    DEBUG("Service %s added to list", service_name);

    fclose(file);

    return strdup("OK");
}

char* get_service(char* file_path, get_params_t* params, size_t get_params_size)
{
    if (get_params_size == 0) {
        WARNING("No parameters given");
        return NULL;
    }

    char* service_name = NULL;
    for (size_t i = 0; i < get_params_size; i++) {
        if (strcmp(params[i].key, "id") == 0) {
            service_name = params[i].val;
            break;
        }
    }

    if (service_name == NULL) {
        WARNING("No service parameter given");
        return NULL;
    }

    DEBUG("Service: %s", service_name);

#ifdef RASPBERRYPI
    char* command = "systemctl status %s --no-page";
    char* command_with_service = malloc(strlen(command) + strlen(service_name) + 1);
    if (command_with_service == NULL) {
        WARNING("Failed to allocate memory for command_with_service");
        return NULL;
    }
    sprintf(command_with_service, command, service_name);
#else
    char* command_with_service = strdup("echo WSL is making things difficult...");
#endif

    char* service = runCommandNoNewLine(command_with_service);
    if (service == NULL) {
        WARNING("Failed to get service %s", service_name);
        return NULL;
    }

    size_t service_len = strlen(service);
    ut_replace_text(&service, &service_len, "\n", "<br>");

    parser_args_t p_args[] = {
        { .key = "service", .fun_cal = NULL, .fun_args = service },
        { .key = "name", .fun_cal = NULL, .fun_args = service_name }
    };

    parser_args_list_t p_args_list = { .args = p_args, .size = 2 };

    char* html = html_parse(file_path, p_args_list);

    free(command_with_service);
    free(service);

    return html;
}

char* remove_from_relevant_server_list(char* service_name)
{
    ut_file_by_line_t* services_file = NULL;

    if ((services_file = ut_file_by_line_open("data/services.data.txt")) == NULL) {
        WARNING("Failed to read file %s: Not found", "data/services.data.txt");
        return strdup("OK");
    }

    char* line;
    char* final_file_text = NULL;
    size_t final_len = 0;

    while ((line = ut_file_by_line_next(services_file)) != NULL) {
        DEBUG("line: %s, %s", line, service_name);

        if (strcmp(line, service_name) == 0) {
            continue;
        }

        size_t line_len = strlen(line) + 1;

        if ((final_file_text = realloc(final_file_text, final_len + line_len + 1)) == NULL) {
            WARNING("Failed to reallocate memory for final_file_text");
            free(final_file_text);
            ut_file_by_line_close(services_file);
            return strdup("ERROR");
        }

        if (final_len == 0) {
            strcpy(final_file_text, line);
        } else {
            strcat(final_file_text, "\n");
            strcat(final_file_text, line);
        }
        final_len += line_len;
    }
    ut_file_by_line_close(services_file);

    FILE* file = fopen("data/services.data.txt", "w");
    if (file == NULL) {
        WARNING("Failed to open file %s", "data/services.data.txt");
        free(final_file_text);
        return strdup("ERROR");
    }

    if (final_file_text != NULL) {
        fprintf(file, "%s\n", final_file_text);
        free(final_file_text);
    } else {
        fprintf(file, "\n");
    }

    fclose(file);

    DEBUG("Service %s removed from list", service_name);
    return strdup("OK");
}

char* restart_service(char* service_name)
{
#ifdef RASPBERRYPI
    char* command = "systemctl restart %s";
#else
    char* command = "echo restarting service %s";
#endif
    char* command_with_service = malloc(strlen(command) + strlen(service_name) + 1);
    if (command_with_service == NULL) {
        WARNING("Failed to allocate memory for command_with_service");
        return NULL;
    }
    sprintf(command_with_service, command, service_name);

    char* response = runCommandNoNewLine(command_with_service);
    if (response == NULL) {
        WARNING("Failed to restart service %s", service_name);
        return NULL;
    }

    DEBUG("Service %s restarted", service_name);

    free(command_with_service);

    return strdup("OK");
}

char* stop_service(char* service_name)
{
#ifdef RASPBERRYPI
    char* command = "systemctl stop %s";
#else
    char* command = "echo stopping service %s";
#endif

    char* command_with_service = malloc(strlen(command) + strlen(service_name) + 1);
    if (command_with_service == NULL) {
        WARNING("Failed to allocate memory for command_with_service");
        return NULL;
    }
    sprintf(command_with_service, command, service_name);

    char* response = runCommandNoNewLine(command_with_service);
    if (response == NULL) {
        WARNING("Failed to stop service %s", service_name);
        return NULL;
    }

    DEBUG("Service %s stopped", service_name);

    free(command_with_service);

    return strdup("OK");
}