#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils.h"

char* _runCommand(const char* command, int remove_new_line)
{
    FILE* fp;
    size_t buffer_size = 1024;
    size_t total_size = 0;
    char* result = NULL;

    char* full_command;
    if (remove_new_line) {
        full_command = calloc(strlen(command) + strlen(" | tr -d '\\n'") + 1, sizeof(char));
        if (full_command == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        sprintf(full_command, "%s | tr -d '\\n'", command);
    } else {
        full_command = strdup(command);
        if (full_command == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }

    fp = popen(full_command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run command\n");
        exit(1);
    }

    result = malloc(buffer_size);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    size_t bytes_read;
    char buffer[1024];
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (total_size + bytes_read >= buffer_size) {
            buffer_size *= 2;
            char* temp = realloc(result, buffer_size);
            if (temp == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                free(result);
                exit(1);
            }
            result = temp;
        }
        memcpy(result + total_size, buffer, bytes_read);
        total_size += bytes_read;
    }

    result[total_size] = '\0';

    pclose(fp);

    free(full_command);

    return result;
}

char* runCommand(const char* command)
{
    return _runCommand(command, 1);
}

char* runCommandNoNewLine(const char* command)
{
    return _runCommand(command, 0);
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