#include "index_controller.h"
#include "../utils.h"
#include "../html_parser.h"
#include "commands.h"

char* index_controller_init(char* file_path)
{

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

    return html_parse(file_path, p_args_list);
}
