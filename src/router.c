#include "router.h"
#include "response.h"
#include "cgi.h"
#include <string.h>

void handle_request(int client_fd, HttpRequest *req) {
    if (!req) return;

    if (strstr(req->uri, "..") != NULL) {
        send_403(client_fd, req->keep_alive);
        return;
    }

    if (strncmp(req->uri, "/cgi-bin/", 9) == 0) {
        char script_path[512] = "public";
        strncat(script_path, req->uri, sizeof(script_path) - strlen(script_path) - 1);
        handle_cgi(client_fd, script_path, req);
    } else if (strcmp(req->method, "POST") == 0 && strcmp(req->uri, "/echo") == 0) {
        send_echo_response(client_fd, req->body, req->content_length, req->keep_alive);
    } else if (strcmp(req->method, "GET") == 0) {
        char file_path[512] = "public";
        if (strcmp(req->uri, "/") == 0) {
            strncat(file_path, "/index.html", sizeof(file_path) - strlen(file_path) - 1);
        } else {
            strncat(file_path, req->uri, sizeof(file_path) - strlen(file_path) - 1);
        }
        send_static_file(client_fd, file_path, req->keep_alive);
    } else {
        send_405(client_fd, req->keep_alive);
    }
}
