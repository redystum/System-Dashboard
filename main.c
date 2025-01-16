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
        WARNING("Debug mode enabled");
    #endif

    #ifdef RASPBERRYPI
        INFO("\nRunning on Raspberry Pi\n");
    #endif

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
        { .key = "disk", .fun_cal = getDiskUsed },
        { .key = "disk_total", .fun_cal = getDiskTotal },
        { .key = "disk_percentage", .fun_cal = getDiskPercentage },
        { .key = "temperature", .fun_cal = getTemperature },
    };

    parser_args_list_t p_args_list = { .args = p_args, .size = sizeof(p_args) / sizeof(p_args[0]) };

    server_init(port, p_args_list);

    cmdline_parser_free(&args);
    return 0;
}

char* runCommand(char* command)
{
    FILE* fp;
    char path[1035];

    char* full_command = calloc(strlen(command) + strlen(" | tr -d '\n'") + 1, sizeof(char));
    if (full_command == NULL) {
        ERROR(1, "Memory allocation failed\n");
        exit(1);
    }
    sprintf(full_command, "%s | tr -d '\n'", command);

    fp = popen(full_command, "r");
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
    char* command = "top -bn1 | grep 'Cpu(s)' | sed 's/.*, *\\([0-9.]*\\)%* id.*/\\1/' | awk '{print 100 - $1}'";
    return runCommand(command);
}

char* getDiskUsed()
{
    #ifdef RASPBERRYPI
        char* command = "df -h | grep /dev/mmcblk0p2 | awk '{print $3}'";
    #else
        char* command = "df -h | grep /dev/sdc | awk '{print $3}'";
    #endif
    return runCommand(command);
}

char* getDiskTotal()
{
    #ifdef RASPBERRYPI
        char* command = "df -h | grep /dev/mmcblk0p2 | awk '{print $2}'";
    #else
        char* command = "df -h | grep /dev/sdc | awk '{print $2}'";
    #endif
    return runCommand(command);
}

char* getDiskPercentage()
{
    #ifdef RASPBERRYPI
        char* command = "df -h | grep /dev/mmcblk0p2 | awk '{print $5}'";
    #else
        char* command = "df -h | grep /dev/sdc | awk '{print $5}'";
    #endif
    return runCommand(command);
}

char* getTemperature()
{
    #ifdef RASPBERRYPI
        char* command = "vcgencmd measure_temp | awk -F '=' '{print $2}' | awk -F \"'\" '{print $1}'";
    #else
        char* command = "echo 45";
    #endif
    return runCommand(command);
}