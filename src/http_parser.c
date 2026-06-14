#include "http_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>

#define BUFFER_SIZE 4096

HttpRequest *parse_http_request(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
        return NULL;
    }
    buffer[bytes_read] = '\0';

    HttpRequest *req = malloc(sizeof(HttpRequest));
    if (!req) return NULL;
    memset(req, 0, sizeof(HttpRequest));

    if (sscanf(buffer, "%15s %255s %15s", req->method, req->uri, req->version) != 3) {
        free(req);
        return NULL;
    }
    printf("[PID %d] Parsed Request - Method: %s, URI: %s, Version: %s\n", getpid(), req->method, req->uri, req->version);

    char *headers_end = strstr(buffer, "\r\n\r\n");
    size_t headers_len = headers_end ? (size_t)(headers_end - buffer) + 4 : (size_t)bytes_read;

    char *line = strstr(buffer, "\r\n");
    if (line) {
        line += 2; // Skip request line
        while (line && req->header_count < MAX_HEADERS) {
            char *next_line = strstr(line, "\r\n");
            if (next_line == line) break; // End of headers
            if (next_line) *next_line = '\0';
            
            char *colon = strchr(line, ':');
            if (colon) {
                *colon = '\0';
                char *key = line;
                char *value = colon + 1;
                while (*value == ' ') value++;
                
                strncpy(req->headers[req->header_count].key, key, sizeof(req->headers[0].key) - 1);
                strncpy(req->headers[req->header_count].value, value, sizeof(req->headers[0].value) - 1);
                req->header_count++;
            }
            if (next_line) line = next_line + 2;
            else break;
        }
    }

    for (int i = 0; i < req->header_count; i++) {
        if (strcasecmp(req->headers[i].key, "Content-Length") == 0) {
            req->content_length = atoi(req->headers[i].value);
            break;
        }
    }

    if (req->content_length > 0) {
        req->body = malloc(req->content_length + 1);
        if (req->body) {
            size_t body_read_so_far = bytes_read - headers_len;
            if (body_read_so_far > 0) {
                memcpy(req->body, buffer + headers_len, body_read_so_far);
            }
            size_t total_body_read = body_read_so_far;
            while (total_body_read < (size_t)req->content_length) {
                ssize_t more_bytes = read(client_fd, req->body + total_body_read, req->content_length - total_body_read);
                if (more_bytes <= 0) break;
                total_body_read += more_bytes;
            }
            req->body[total_body_read] = '\0';
        }
    }

    return req;
}

void free_http_request(HttpRequest *req) {
    if (!req) return;
    if (req->body) free(req->body);
    free(req);
}
