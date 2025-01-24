#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

/*
 * Prints an error message and aborts the program
 *
 * @param code the error code
 * @param fmt the format string
 */
#define ERROR(code, ...) error(code, __VA_ARGS__)
/*
 * Prints a warning message
 *
 * @param fmt the format string
 */
#define WARNING(...) warning(__VA_ARGS__)
/*
 * Prints an info message
 *
 * @param fmt the format string
 */
#define INFO(...) info(__VA_ARGS__)

/*
 * Prints a debug message if the DEBUG_ENABLED macro is defined
 *
 * @param fmt the format string
 */
#ifdef DEBUG_ENABLED
#define DEBUG(...) debug(__FILE__, __LINE__, __VA_ARGS__)
#else
#define DEBUG(...)
#endif

/*
 * Initializes the log file
 *
 * @param path the path of the log file
 */
void ut_file_log_init(const char* path);

/*
 * Returns the path of the log file
 *
 * @return the path of the log file
 */
char* ut_get_file_log_path();

/*
 * Frees the memory related to the log file
 */
void ut_file_log_close();

/*
 * Logs a message to the log file
 *
 * @param text the message to log
 */
#define EOL '\n'

/*
 * Represents a slice of a string
 */
typedef struct {
    char* str;
    unsigned int len;
} ut_string_slice_t;

/*
 * Copies the content of a string slice to a new string
 *
 * @param str_slice the string slice to copy
 * @param str the string to store the copied content
 */
void ut_string_slice_original(ut_string_slice_t* str_slice, char** str);

/*
 * Reads a file and stores its content in a buffer
 *
 * @param file_name the name of the file to read
 * @param buffer the buffer to store the file content
 *
 * @return the length of the file or -1 if an error occurred
 */
int ut_read_file(const char* file_name, char** buffer);

/*
 * Represents a file read line by line
 */
typedef struct {
    FILE* file;
    char* buffer;
    size_t buffer_size;
    size_t buffer_len;
} ut_file_by_line_t;

/*
 * Opens a file to read it line by line
 *
 * @param file_name the name of the file to open
 *
 * @return the file read line by line or NULL if an error occurred
 */
ut_file_by_line_t* ut_file_by_line_open(const char* file_name);

/*
 * Reads the next line of a file
 *
 * @param file_by_line the file read line by line
 *
 * @return the next line of the file or NULL if the end of the file is reached
 */
char* ut_file_by_line_next(ut_file_by_line_t* file_by_line);

/*
 * Closes a file read line by line
 *
 * @param file_by_line the file read line by line
 */
void ut_file_by_line_close(ut_file_by_line_t* file_by_line);

/*
 * Represents a dynamic array
 */
typedef struct {
    void* data;
    unsigned int len; // number of elements in the array
    unsigned int cap;
    size_t size; // size of the elements in the array
} ut_dynamic_array_t;

/*
 * Initializes a dynamic array
 *
 * @param arr the dynamic array to initialize
 * @param size the size of the elements in the array
 */
void ut_array_init(ut_dynamic_array_t* arr, size_t elem_size);
void ut_array_push(ut_dynamic_array_t* arr, void* elem);
void* ut_array_get(ut_dynamic_array_t* arr, size_t index);
void ut_array_free(ut_dynamic_array_t* arr);

/*
 * Concatenates strings
 *
 * @param dest the destination string
 * @param ... the strings to concatenate
 */
void ut_str_cat(char** dest, ...);

/*
 * Trims a string
 *
 * @param str the string to trim
 */
void ut_trim(char* str);

/*
 * Replaces a text in a string
 *
 * @param logs the string to replace the text
 * @param len the length of the string
 * @param old_text the text to replace
 * @param new_text the new text
 */
void ut_replace_text(char **logs, size_t *len, const char *old_text, const char *new_text);

void error(int code, char* fmt, ...);
void warning(char* format, ...);
void info(char* format, ...);
void debug(char* file, int line, char* format, ...);

#endif // UTILS_H