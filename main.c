#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "args.h"
#include "server.h"
#include "utils.h"

#include "controllers/index_controller.h"

#define DEFAULT_PORT 8080

int main(int argc, char* argv[])
{
    struct gengetopt_args_info args;

    if (cmdline_parser(argc, argv, &args) != 0) {
        ERROR(1, "Error parsing command line");
        return 1;
    }

    ut_file_log_init("log.txt");

    #ifdef DEBUG_ENABLED
        WARNING("Debug mode enabled\n");
    #endif

    #ifdef RASPBERRYPI
        INFO("Running on Raspberry Pi\n");
    #endif

    int port = DEFAULT_PORT;
    if (args.port_given == 1) {
        port = args.port_arg;
    } else {
        INFO("No port given, using default = %d", DEFAULT_PORT);
    }

    size_t controllers_size = 1;

    controller_t* controllers = malloc(sizeof(controller_t) * controllers_size);
    if (!controllers) {
        ERROR(1, "Failed to allocate memory for controllers");
    }

    controllers[0] = (controller_t) { .file = "index.html", .fun = index_controller_init };

    controller_list_t controllers_list = {
        .controllers = controllers,
        .size = controllers_size
    };

    server_init(port, controllers_list);

    ut_file_log_close();

    cmdline_parser_free(&args);
    return 0;
}