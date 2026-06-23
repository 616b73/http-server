#include "response.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

const char *get_mime_type(const char *file_path) {
    const char *ext = strrchr(file_path, '.');
    if (!ext) return "application/octet-stream";
    
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    
    return "application/octet-stream";
}

void send_404(int client_fd, const char *file_path, int keep_alive) {
    char response[256];
    snprintf(response, sizeof(response),
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "Connection: %s\r\n"
        "\r\n"
        "404 Not Found", keep_alive ? "keep-alive" : "close");
    write(client_fd, response, strlen(response));
    printf("[PID %d] Served 404 Not Found for %s\n", getpid(), file_path);
}

void send_403(int client_fd, int keep_alive) {
    char response[256];
    snprintf(response, sizeof(response),
        "HTTP/1.1 403 Forbidden\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "Connection: %s\r\n"
        "\r\n"
        "403 Forbidden", keep_alive ? "keep-alive" : "close");
    write(client_fd, response, strlen(response));
    printf("[PID %d] Served 403 Forbidden\n", getpid());
}

void send_405(int client_fd, int keep_alive) {
    char response[256];
    snprintf(response, sizeof(response),
        "HTTP/1.1 405 Method Not Allowed\r\n"
        "Content-Length: 0\r\n"
        "Connection: %s\r\n"
        "\r\n", keep_alive ? "keep-alive" : "close");
    write(client_fd, response, strlen(response));
}

void send_static_file(int client_fd, const char *file_path, int keep_alive) {
    int file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
        send_404(client_fd, file_path, keep_alive);
        return;
    }

    struct stat file_stat;
    fstat(file_fd, &file_stat);
    off_t file_size = file_stat.st_size;

    const char *mime_type = get_mime_type(file_path);

    char header_buf[256];
    snprintf(header_buf, sizeof(header_buf),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: %s\r\n"
             "\r\n", mime_type, (long)file_size, keep_alive ? "keep-alive" : "close");
    
    write(client_fd, header_buf, strlen(header_buf));

    char file_buffer[1024];
    ssize_t bytes_read_from_file;
    while ((bytes_read_from_file = read(file_fd, file_buffer, sizeof(file_buffer))) > 0) {
        write(client_fd, file_buffer, bytes_read_from_file);
    }
    close(file_fd);
    printf("[PID %d] Served %s (%ld bytes)\n", getpid(), file_path, (long)file_size);
}

void send_echo_response(int client_fd, const char *body, int content_length, int keep_alive) {
    char header_buf[256];
    snprintf(header_buf, sizeof(header_buf),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/plain\r\n"
             "Content-Length: %d\r\n"
             "Connection: %s\r\n"
             "\r\n", content_length, keep_alive ? "keep-alive" : "close");
    write(client_fd, header_buf, strlen(header_buf));
    if (body) {
        write(client_fd, body, content_length);
    }
    printf("[PID %d] Handled POST /echo (%d bytes)\n", getpid(), content_length);
}
