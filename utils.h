#ifndef UTILS_H
#define UTILS_H

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
void ut_array_init(ut_dynamic_array_t* arr, size_t size);
void ut_array_push(ut_dynamic_array_t* arr, void* elem);
void* ut_array_get(ut_dynamic_array_t* arr, unsigned int index);
void ut_array_free(ut_dynamic_array_t* arr);

/*
 * Concatenates strings
 *
 * @param dest the destination string
 * @param ... the strings to concatenate
 */
void ut_str_cat(char **dest, ...);

/*
 * Trims a string
 *
 * @param str the string to trim
 */
void ut_trim(char* str);

void error(int code, char* fmt, ...);
void warning(char* format, ...);
void info(char* format, ...);
void debug(char* file, int line, char* format, ...);

#endif // UTILS_H