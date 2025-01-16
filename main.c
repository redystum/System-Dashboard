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

#define DEFAULT_PORT 8080

char* runCommand(char* command);
char* getRamUsed();
char* getRamTotal();
char* getCpuUsed();
char* getDiskUsed();
char* getDiskTotal();
char* getDiskPercentage();
char* getTemperature();

int main(int argc, char* argv[])
{
    struct gengetopt_args_info args;

    if (cmdline_parser(argc, argv, &args) != 0) {
        ERROR(1, "Error parsing command line");
        return 1;
    }

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

    server_init(port);

    cmdline_parser_free(&args);
    return 0;
}