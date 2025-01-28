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

controller_list_t _controllers;

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
    } else if (strcmp(file_ext, "js") == 0) {
        return "application/javascript";
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

int send200(int client_fd, const char* content_type, const char* content, size_t content_length)
{
    char header[BUFFER_SIZE];
    int header_len = snprintf(header, BUFFER_SIZE,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "\r\n",
        content_type, content_length);

    send(client_fd, header, header_len, 0);
    send(client_fd, content, content_length, 0);

    return 200;
}

int send400(int client_fd)
{
    const char* bad_request_response = "HTTP/1.1 400 Bad Request\r\n"
                                       "Content-Type: text/plain\r\n"
                                       "Content-Length: 11\r\n"
                                       "\r\n"
                                       "Bad Request";
    send(client_fd, bad_request_response, strlen(bad_request_response), 0);

    return 400;
}

int send404(int client_fd)
{
    const char* not_found_response = "HTTP/1.1 404 Not Found\r\n"
                                     "Content-Type: text/plain\r\n"
                                     "Content-Length: 13\r\n"
                                     "\r\n"
                                     "404 Not Found";
    send(client_fd, not_found_response, strlen(not_found_response), 0);

    return 404;
}

int send500(int client_fd)
{
    const char* internal_server_error_response = "HTTP/1.1 500 Internal Server Error\r\n"
                                                 "Content-Type: text/plain\r\n"
                                                 "Content-Length: 21\r\n"
                                                 "\r\n"
                                                 "Internal Server Error";
    send(client_fd, internal_server_error_response, strlen(internal_server_error_response), 0);

    return 500;
}

int send_http_response(int client_fd, const char* file_name, const char* file_ext)
{
    char file_path[BUFFER_SIZE];
    snprintf(file_path, BUFFER_SIZE, "static/%s", file_name);

    if (strcmp(file_ext, "html") == 0) {
        char* html_content = NULL;

        for (size_t i = 0; i < _controllers.size; i++) {
            controller_t controller = _controllers.controllers[i];
            if (strcmp(controller.file, file_name) == 0) {
                html_content = controller.fun(file_path);
                break;
            }
        }

        if (!html_content) {
            free(html_content);
            return send404(client_fd);
        }

        send200(client_fd, "text/html", html_content, strlen(html_content));

        free(html_content);
        return 200;
    }

    int file_fd = open(file_path, O_RDONLY);
    if (file_fd == -1) {
        return send404(client_fd);
    }

    struct stat file_stat;
    fstat(file_fd, &file_stat);
    off_t file_size = file_stat.st_size;

    send200(client_fd, get_mime_type(file_ext), NULL, file_size);

    off_t offset = 0;
    while (offset < file_size) {
        ssize_t sent = sendfile(client_fd, file_fd, &offset, file_size - offset);
        if (sent <= 0) {
            WARNING("sendfile failed");
            break;
        }
    }

    close(file_fd);
    return 200;
}

int send_post_response(int client_fd, const char* file_name, char* buffer, ssize_t bytes_received)
{
    char* content_length_start = strstr(buffer, "Content-Length: ");
    if (!content_length_start) {
        return send400(client_fd);
    }

    content_length_start += 16; // Skip "Content-Length: "
    char* content_length_end = strchr(content_length_start, '\r');
    if (!content_length_end) {
        return send400(client_fd);
    }

    char cl_str[32];
    strncpy(cl_str, content_length_start, content_length_end - content_length_start);
    cl_str[content_length_end - content_length_start] = '\0';
    size_t content_length = atoi(cl_str);

    char* body_start = strstr(buffer, "\r\n\r\n");
    if (!body_start) {
        return send400(client_fd);
    }
    body_start += 4; // Skip "\r\n\r\n"

    char* body = malloc(content_length + 1);
    size_t body_read = bytes_received - (body_start - buffer);
    memcpy(body, body_start, body_read);
    size_t total_read = body_read;

    while (total_read < content_length) {
        ssize_t n = recv(client_fd, body + total_read, content_length - total_read, 0);
        if (n <= 0) {
            free(body);
            return send500(client_fd);
        }
        total_read += n;
    }
    body[content_length] = '\0';

    DEBUG("POST body: %s", body);

    char* response_body = NULL;

    for (size_t i = 0; i < _controllers.size; i++) {
        controller_t controller = _controllers.controllers[i];
        if (strcmp(controller.file, file_name) == 0) {
            response_body = controller.fun(body);
            break;
        }
    }

    if (!response_body) {
        free(response_body);
        return send404(client_fd);
    }

    if (strcmp(response_body, "ERROR") == 0)
    {
        free(response_body);
        return send500(client_fd);
    }

    send200(client_fd, "text/plain", response_body, strlen(response_body));

    free(body);
    free(response_body);
    return 200;
}

void* handle_client(void* arg)
{
    int client_fd = *((int*)arg);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        close(client_fd);
        return NULL;
    }

    int status = 400;
    buffer[bytes_received] = '\0';

    regex_t regex;
    regcomp(&regex, "^(GET|POST) /([^ ]*) HTTP/1", REG_EXTENDED);
    regmatch_t matches[3];

    if (regexec(&regex, buffer, 3, matches, 0) == 0) {
        char method[16];
        char url_path[BUFFER_SIZE];

        snprintf(method, sizeof(method), "%.*s", matches[1].rm_eo - matches[1].rm_so, buffer + matches[1].rm_so);
        snprintf(url_path, sizeof(url_path), "%.*s", matches[2].rm_eo - matches[2].rm_so, buffer + matches[2].rm_so);

        char* decoded_path = url_decode(url_path);
        char* file_name = decoded_path;

        if (strcmp(method, "GET") == 0 && strlen(file_name) == 0) {
            free(file_name);
            file_name = strdup("index.html");
        }

        if (strcmp(method, "POST") == 0) {
            status = send_post_response(client_fd, file_name, buffer, bytes_received);
        } else { // GET
            const char* file_ext = get_file_extension(file_name);
            status = send_http_response(client_fd, file_name, file_ext);
        }

        free(file_name);
    } else {
        status = send400(client_fd);
    }

    char* first_line_end = strstr(buffer, "\r\n");
    if (first_line_end) {
        *first_line_end = '\0';
    }

    INFO("Client disconnected with status %d, request: %s", status, buffer);
    regfree(&regex);
    close(client_fd);
    return NULL;
}

int server_init(int port, controller_list_t controllers)
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

    _controllers = controllers;

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
        if (pthread_create(&thread_id, NULL, handle_client, client_fd) != 0) {
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
