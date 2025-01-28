#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define COLOR_RED "\x1b[31m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_PURPLE "\x1b[35m"
#define COLOR_RESET "\x1b[0m"

int can_log = 0;
void _file_log(char* tag, char* text, va_list args);

void error(int code, char* fmt, ...)
{
    va_list args, args_copy;

    va_start(args, fmt);
    va_copy(args_copy, args);
    fprintf(stderr, COLOR_RED "[E]\t ");
    vfprintf(stderr, fmt, args);
    _file_log("[E]\t ", fmt, args_copy);
    va_end(args_copy);
    va_end(args);
    fprintf(stderr, "\nAborting\n" COLOR_RESET);
    exit(code);
}

void warning(char* format, ...)
{
    va_list args, args_copy;

    va_start(args, format);
    va_copy(args_copy, args);
    fprintf(stderr, COLOR_YELLOW "[W]\t ");
    vfprintf(stderr, format, args);
    _file_log("[W]\t ", format, args_copy);
    va_end(args_copy);
    va_end(args);
    fprintf(stderr, "\n" COLOR_RESET);
    fflush(stderr);
}

void info(char* format, ...)
{
    va_list args, args_copy;

    va_start(args, format);
    va_copy(args_copy, args);
    fprintf(stdout, COLOR_BLUE "[I]\t ");
    vfprintf(stdout, format, args);
    _file_log("[I]\t ", format, args_copy);
    va_end(args_copy);
    va_end(args);
    fprintf(stdout, "\n" COLOR_RESET);
    fflush(stdout);
}

void debug(char* file, int line, char* format, ...)
{
    va_list args;

    va_start(args, format);
    fprintf(stdout, COLOR_PURPLE "[D %s:%d]: ", file, line);
    vfprintf(stdout, format, args);
    va_end(args);
    fprintf(stdout, "\n" COLOR_RESET);
    fflush(stdout);
}

char* log_file_path = NULL;

void ut_file_log_init(const char* path)
{
    if (path == NULL) {
        can_log = 0;
        return;
    }

    log_file_path = strdup(path);
    if (log_file_path == NULL) {
        error(1, "Error allocating memory for log file path");
    }

    FILE* file = fopen(log_file_path, "w");
    if (file == NULL) {
        error(1, "Error opening log file");
    }
    fclose(file);

    info("Log file initialized at %s", log_file_path);
    can_log = 1;
}

void _file_log(char* tag, char* text, va_list args)
{
    if (can_log == 0) {
        return;
    }

    if (log_file_path == NULL) {
        error(1, "Log file path not initialized");
    }
    FILE* file = fopen(log_file_path, "a");
    if (file == NULL) {
        error(1, "Error opening log file");
    }

    fprintf(file, "%s", tag);
    vfprintf(file, text, args);
    fprintf(file, "\n");

    fclose(file);
}

char* ut_get_file_log_path()
{
    return log_file_path;
}

void ut_file_log_close()
{
    if (log_file_path != NULL) {
        free(log_file_path);
    }
}

void ut_string_slice_original(ut_string_slice_t* str_slice, char** str)
{
    *str = malloc(sizeof(char) * (str_slice->len) + 1);
    if (*str == NULL) {
        error(1, "Error allocating memory for string slice");
    }

    memcpy(*str, str_slice->str, sizeof(char) * (str_slice->len));
    (*str)[str_slice->len] = '\0';
}

int ut_read_file(const char* file_name, char** buffer)
{
    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    *buffer = malloc(length + 1);
    if (*buffer == NULL) {
        fclose(file);
        return -1;
    }

    fread(*buffer, 1, length, file);
    fclose(file);
    (*buffer)[length] = '\0';

    return length;
}

ut_file_by_line_t* ut_file_by_line_open(const char* file_name)
{
    ut_file_by_line_t* file_by_line = malloc(sizeof(ut_file_by_line_t));
    if (file_by_line == NULL) {
        return NULL;
    }

    file_by_line->file = fopen(file_name, "r");
    if (file_by_line->file == NULL) {
        free(file_by_line);
        return NULL;
    }

    file_by_line->buffer = NULL;
    file_by_line->buffer_size = 0;
    file_by_line->buffer_len = 0;

    return file_by_line;
}

char* ut_file_by_line_next(ut_file_by_line_t* file_by_line)
{
    if (file_by_line->file == NULL) {
        return NULL;
    }

    if (file_by_line->buffer == NULL || file_by_line->buffer_len == 0) {
        if (file_by_line->buffer != NULL) {
            free(file_by_line->buffer);
            file_by_line->buffer = NULL;
        }

        ssize_t read = getline(&file_by_line->buffer, &file_by_line->buffer_size, file_by_line->file);
        if (read == -1) {
            return NULL;
        }

        file_by_line->buffer_len = read;
    }

    while (isspace((unsigned char)file_by_line->buffer[0])) {
        memmove(file_by_line->buffer, file_by_line->buffer + 1, file_by_line->buffer_len);
        file_by_line->buffer_len--;
    }

    if (file_by_line->buffer[0] == '#' || file_by_line->buffer_len == 0) {
        file_by_line->buffer_len = 0;
        return ut_file_by_line_next(file_by_line);
    }

    char* line = malloc(file_by_line->buffer_len + 1);
    if (line == NULL) {
        return NULL;
    }

    memcpy(line, file_by_line->buffer, file_by_line->buffer_len);
    line[file_by_line->buffer_len] = '\0';

    // Remove trailing whitespace characters
    char* end = line + file_by_line->buffer_len - 1;
    while (end > line && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';

    // Skip empty lines
    if (strlen(line) == 0) {
        free(line);
        return ut_file_by_line_next(file_by_line);
    }

    file_by_line->buffer_len = 0;

    return line;
}

void ut_file_by_line_close(ut_file_by_line_t* file_by_line)
{
    if (file_by_line->file != NULL) {
        fclose(file_by_line->file);
    }

    if (file_by_line->buffer != NULL) {
        free(file_by_line->buffer);
    }

    free(file_by_line);
}

void ut_array_init(ut_dynamic_array_t* arr, size_t elem_size)
{
    arr->len = 0;
    arr->cap = 1;
    arr->size = elem_size;
    arr->data = malloc(arr->cap * elem_size);
}

void ut_array_push(ut_dynamic_array_t* arr, void* elem)
{
    if (arr->len == arr->cap) {
        arr->cap *= 2;
        arr->data = realloc(arr->data, arr->cap * arr->size);
    }
    memcpy((char*)arr->data + arr->len * arr->size, elem, arr->size);
    arr->len++;
}

void* ut_array_get(ut_dynamic_array_t* arr, size_t index)
{
    if (index >= arr->len)
        return NULL;
    return (char*)arr->data + index * arr->size;
}

void ut_array_free(ut_dynamic_array_t* arr)
{
    free(arr->data);
    arr->len = 0;
    arr->cap = 0;
    arr->data = NULL;
}

void ut_str_cat(char** dest, ...)
{
    va_list args;
    va_start(args, dest);

    size_t initial_length = (*dest != NULL) ? strlen(*dest) : 0;
    size_t total_length = initial_length;
    char* src;
    while ((src = va_arg(args, char*)) != NULL) {
        total_length += strlen(src);
    }
    va_end(args);

    char* new_dest = realloc(*dest, total_length + 1);
    if (new_dest == NULL) {
        error(1, "Error reallocating memory for concatenated string");
    }
    *dest = new_dest;

    va_start(args, dest);
    if (initial_length == 0) {
        (*dest)[0] = '\0';
    }
    while ((src = va_arg(args, char*)) != NULL) {
        strcat(*dest, src);
    }
    va_end(args);

    ut_trim(*dest);
}

void ut_trim(char* str)
{
    char* end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) {
        end--;
    }
    *(end + 1) = '\0';

    while (*str && isspace(*str)) {
        str++;
    }
}

void ut_replace_text(char **logs, size_t *len, const char *old_text, const char *new_text) {
    size_t old_len = strlen(old_text);
    size_t new_len = strlen(new_text);

    size_t count = 0;
    for (char *pos = *logs; (pos = strstr(pos, old_text)) != NULL; pos += old_len) {
        count++;
    }

    size_t result_len = *len + count * (new_len - old_len);
    char *result = malloc(result_len + 1); // +1 for null terminator

    if (!result) {
        error(1, "Failed to allocate memory");
    }

    char *src = *logs;
    char *dst = result;
    while ((src = strstr(src, old_text)) != NULL) {
        size_t prefix_len = src - *logs;
        strncpy(dst, *logs, prefix_len);
        dst += prefix_len;

        strcpy(dst, new_text);
        dst += new_len;

        src += old_len;
        *logs = src;
    }

    strcpy(dst, *logs);

    *logs = result;
    *len = result_len;
}