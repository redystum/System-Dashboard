#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "server.h"
#include "utils.h"

#define BUFFER_SIZE 8192

typedef struct {
    parser_args_list_t html_parser_args;
    int* client_fd;
} thread_args_t;

const char* get_file_extension(const char* file_name)
{
    const char* dot = strrchr(file_name, '.');
    if (!dot || dot == file_name) {
        return "";
    }
    return dot + 1;
}

const char* get_mime_type(const char* file_ext)
{
    if (strcmp(file_ext, "html") == 0 || strcmp(file_ext, "htm") == 0) {
        return "text/html";
    } else if (strcmp(file_ext, "txt") == 0) {
        return "text/plain";
    } else if (strcmp(file_ext, "jpg") == 0 || strcmp(file_ext, "jpeg") == 0) {
        return "image/jpeg";
    } else if (strcmp(file_ext, "png") == 0) {
        return "image/png";
    } else if (strcmp(file_ext, "css") == 0) {
        return "text/css";
    } else {
        return "application/octet-stream";
    }
}

char* url_decode(const char* src)
{
    size_t src_len = strlen(src);
    char* decoded = malloc(src_len + 1);
    size_t decoded_len = 0;

    for (size_t i = 0; i < src_len; i++) {
        if (src[i] == '%' && i + 2 < src_len) {
            unsigned int hex_val;
            sscanf(src + i + 1, "%2x", &hex_val);
            decoded[decoded_len++] = (char)hex_val;
            i += 2;
        } else {
            decoded[decoded_len++] = src[i];
        }
    }

    decoded[decoded_len] = '\0';
    return decoded;
}

void send_http_response(int client_fd, const char* file_name, const char* file_ext, parser_args_list_t p_args)
{
    if (strcmp(file_ext, "html") == 0) {
        char* html_content = html_parse(file_name, p_args);
        if (!html_content) {
            const char* not_found_response = "HTTP/1.1 404 Not Found\r\n"
                                             "Content-Type: text/plain\r\n"
                                             "Content-Length: 13\r\n"
                                             "\r\n"
                                             "404 Not Found";
            send(client_fd, not_found_response, strlen(not_found_response), 0);
            return;
        }

        char header[BUFFER_SIZE];
        int header_len = snprintf(header, BUFFER_SIZE,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %ld\r\n"
            "\r\n",
            strlen(html_content));

        send(client_fd, header, header_len, 0);
        send(client_fd, html_content, strlen(html_content), 0);

        free(html_content);
    } else {
        int file_fd = open(file_name, O_RDONLY);
        if (file_fd == -1) {
            const char* not_found_response = "HTTP/1.1 404 Not Found\r\n"
                                             "Content-Type: text/plain\r\n"
                                             "Content-Length: 13\r\n"
                                             "\r\n"
                                             "404 Not Found";
            send(client_fd, not_found_response, strlen(not_found_response), 0);
            return;
        }

        struct stat file_stat;
        fstat(file_fd, &file_stat);
        off_t file_size = file_stat.st_size;

        char header[BUFFER_SIZE];
        int header_len = snprintf(header, BUFFER_SIZE,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "\r\n",
            get_mime_type(file_ext), file_size);

        send(client_fd, header, header_len, 0);

        off_t offset = 0;
        while (offset < file_size) {
            ssize_t sent = sendfile(client_fd, file_fd, &offset, file_size - offset);
            if (sent <= 0) {
                WARNING("sendfile");
                break;
            }
        }

        close(file_fd);
    }
}

void* handle_client(void* arg)
{
    thread_args_t* t_args = (thread_args_t*)arg;
    int client_fd = *(t_args->client_fd);
    free(t_args->client_fd);
    parser_args_list_t p_args = t_args->html_parser_args;
    free(t_args);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        close(client_fd);
        return NULL;
    }

    buffer[bytes_received] = '\0';

    regex_t regex;
    regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
    regmatch_t matches[2];

    if (regexec(&regex, buffer, 2, matches, 0) == 0) {
        buffer[matches[1].rm_eo] = '\0';
        const char* url_encoded_file_name = buffer + matches[1].rm_so;
        char* file_name = url_decode(url_encoded_file_name);

        const char* file_ext = get_file_extension(file_name);
        send_http_response(client_fd, file_name, file_ext, p_args);

        free(file_name);
    } else {
        const char* bad_request_response = "HTTP/1.1 400 Bad Request\r\n"
                                           "Content-Type: text/plain\r\n"
                                           "Content-Length: 11\r\n"
                                           "\r\n"
                                           "Bad Request";
        send(client_fd, bad_request_response, strlen(bad_request_response), 0);
    }

    regfree(&regex);
    close(client_fd);
    return NULL;
}

int server_init(int port, parser_args_list_t p_args)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        WARNING("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)
    };

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        WARNING("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        WARNING("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    INFO("Server listening on port %d\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int* client_fd = malloc(sizeof(int));
        if (!client_fd) {
            WARNING("malloc failed");
            continue;
        }

        *client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (*client_fd < 0) {
            WARNING("accept failed");
            free(client_fd);
            continue;
        }

        pthread_t thread_id;
        thread_args_t* t_args = malloc(sizeof(thread_args_t));
        if (!t_args) {
            WARNING("malloc failed");
            close(*client_fd);
            free(client_fd);
            continue;
        }
        t_args->client_fd = client_fd;
        t_args->html_parser_args = p_args;
        if (pthread_create(&thread_id, NULL, handle_client, t_args) != 0) {
            WARNING("pthread_create failed");
            close(*client_fd);
            free(client_fd);
        } else {
            pthread_detach(thread_id);
        }
    }

    close(server_fd);
    return 0;
}
