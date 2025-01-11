#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "args.h"
#include "utils.h"
#include "server.h"

#define DEFAULT_PORT 8080

void replace(char *str, char *orig, char *rep);
bool starts_with(const char *a, const char *b);
ssize_t find_str(const char *str, const char *substr);

int main(int argc, char *argv[]) {
    struct gengetopt_args_info args;

    if (cmdline_parser(argc, argv, &args) != 0) {
        ERROR(1, "Error parsing command line");
        return 1;
    }

    int port = DEFAULT_PORT;
    if (args.port_given == 1) {
        port = args.port_arg;
    } else {
        INFO("No port given, using default = %d", DEFAULT_PORT);
    }
    
    server_init(port);

    /*
    char *buffer = NULL;

    int length = 0;
    if ((length = ut_read_file(input, &buffer)) == -1) {
        ERROR(1, "Error reading input file");
    }
    printf("Read %d bytes from %s\n", length, input);
    */


    cmdline_parser_free(&args);
    return 0;
}
