#ifndef SERVICES_CONTROLLER_H
#define SERVICES_CONTROLLER_H

#include "../server.h"

char* services_controller_init(char* file_path);
char* add_to_relevant_server_list(char* service_name);
char* get_service(char* file_path, get_params_t* params, size_t get_params_size);

#endif