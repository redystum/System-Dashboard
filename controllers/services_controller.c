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
    services = runCommand("systemctl list-units --type=service --no-legend --no-pager --output=json | jq -r '.[] | \"\\(.unit)|\\(.sub)|\\(.description)\"'");
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
        return NULL;
    }

    p_args[0] = (parser_args_t) { .key = "services", .fun_cal = NULL, .fun_args = &services_array };
    parser_args_list_t p_args_list = { .args = p_args, .size = 1 };

    char* html = html_parse(file_path, p_args_list);

    for (size_t i = 0; i < services_array.len; i++) {
        free(*(parser_args_t**)ut_array_get(&services_array, i));
    }

    ut_array_free(&services_array);

    return html;
}