#include "router.h"
#include "response.h"
#include <string.h>

void handle_request(int client_fd, HttpRequest *req) {
    if (!req) return;

    if (strcmp(req->method, "POST") == 0 && strcmp(req->uri, "/echo") == 0) {
        send_echo_response(client_fd, req->body, req->content_length);
    } else if (strcmp(req->method, "GET") == 0) {
        char file_path[512] = "public";
        if (strcmp(req->uri, "/") == 0) {
            strncat(file_path, "/index.html", sizeof(file_path) - strlen(file_path) - 1);
        } else {
            strncat(file_path, req->uri, sizeof(file_path) - strlen(file_path) - 1);
        }
        send_static_file(client_fd, file_path);
    } else {
        send_405(client_fd);
    }
}
