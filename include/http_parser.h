#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <sys/types.h>

#define MAX_HEADERS 64

typedef struct {
    char key[128];
    char value[512];
} HttpHeader;

typedef struct {
    char method[16];
    char uri[256];
    char version[16];
    HttpHeader headers[MAX_HEADERS];
    int header_count;
    char *body;
    int content_length;
} HttpRequest;

HttpRequest *parse_http_request(int client_fd);
void free_http_request(HttpRequest *req);

#endif
