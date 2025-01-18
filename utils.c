#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"

#define COLOR_RED "\x1b[31m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_PURPLE "\x1b[35m"
#define COLOR_RESET "\x1b[0m"

void error(int code, char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fprintf(stderr, COLOR_RED "[E]\t ");
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\nAborting\n" COLOR_RESET);
    exit(code);
}

void warning(char* format, ...)
{
    va_list args;

    va_start(args, format);
    fprintf(stderr, COLOR_YELLOW "[W]\t ");
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n" COLOR_RESET);
    fflush(stderr);
}

void info(char* format, ...)
{
    va_list args;

    va_start(args, format);
    fprintf(stdout, COLOR_BLUE "[I]\t ");
    vfprintf(stdout, format, args);
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

    if (file_by_line->buffer[0] == '#') {
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

void ut_array_init(ut_dynamic_array_t* arr, size_t size)
{
    arr->size = size;
    arr->len = 0;
    arr->data = NULL;
    arr->cap = 0;
}

void ut_array_push(ut_dynamic_array_t* arr, void* elem)
{
    if (arr->len == arr->cap) {
        if (arr->cap == 0) {
            arr->cap = 1;
        } else {
            arr->cap *= 2;
        }

        arr->data = realloc(arr->data, arr->cap * arr->size);
        if (arr->data == NULL) {
            error(1, "Error reallocating memory for dynamic array");
        }
    }

    memcpy((char*)arr->data + arr->len * arr->size, elem, arr->size);
    arr->len++;
}

void* ut_array_get(ut_dynamic_array_t* arr, unsigned int index)
{
    if (index >= arr->len) {
        return NULL;
    }

    return (char*)arr->data + index * arr->size;
}

void ut_array_free(ut_dynamic_array_t* arr)
{
    free(arr->data);
    arr->data = NULL;
    arr->len = 0;
    arr->cap = 0;
}

void ut_str_cat(char **dest, ...)
{
    va_list args;
    va_start(args, dest);

    size_t initial_length = (*dest != NULL) ? strlen(*dest) : 0;
    size_t total_length = initial_length;
    char *src;
    while ((src = va_arg(args, char *)) != NULL) {
        total_length += strlen(src);
    }
    va_end(args);

    char *new_dest = realloc(*dest, total_length + 1);
    if (new_dest == NULL) {
        error(1, "Error reallocating memory for concatenated string");
    }
    *dest = new_dest;

    va_start(args, dest);
    if (initial_length == 0) {
        (*dest)[0] = '\0';
    }
    while ((src = va_arg(args, char *)) != NULL) {
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