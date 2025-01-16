#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "args.h"
#include "html_parser.h"
#include "server.h"
#include "utils.h"

#define DEFAULT_PORT 8080

char* getRamUsed();
char* getRamTotal();
char* getCpuUsed();
char* runCommand(char* command);

int main(int argc, char* argv[])
{
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

    parser_args_t p_args[] = {
        { .key = "cpu", .fun_cal = getCpuUsed },
        { .key = "ram_total", .fun_cal = getRamTotal },
        { .key = "ram", .fun_cal = getRamUsed },
        { .key = NULL, .fun_cal = NULL },
    };

    parser_args_list_t p_args_list = { .args = p_args, .size = sizeof(p_args) / sizeof(p_args[0]) };

    server_init(port, p_args_list);

    cmdline_parser_free(&args);
    return 0;
}

char* getRamUsed()
{
    char* command = "free -m | grep Mem | awk '{print $3}'";
    return runCommand(command);
}

char* getRamTotal()
{
    char* command = "free -m | grep Mem | awk '{print $2}'";
    return runCommand(command);
}

char* getCpuUsed()
{
    char* command = "top -bn1 | grep 'Cpu(s)' | sed 's/.*, *\\([0-9.]*\\)%* id.*/\\1/' | awk '{print 100 - $1\"%\"}'";
    return runCommand(command);
}

char* runCommand(char* command)
{
    FILE* fp;
    char path[1035];
    fp = popen(command, "r");
    if (fp == NULL) {
        ERROR(1, "Failed to run command\n");
        exit(1);
    }
    fgets(path, sizeof(path) - 1, fp);
    pclose(fp);
    char* result = strdup(path);
    if (result == NULL) {
        ERROR(1, "Memory allocation failed\n");
        exit(1);
    }
    return result;
}