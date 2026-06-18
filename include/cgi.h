#ifndef CGI_H
#define CGI_H

#include "http_parser.h"

void handle_cgi(int client_fd, const char *script_path, HttpRequest *req);

#endif
