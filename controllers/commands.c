#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils.h"

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