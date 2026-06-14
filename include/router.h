#ifndef ROUTER_H
#define ROUTER_H

#include "http_parser.h"

void handle_request(int client_fd, HttpRequest *req);

#endif
