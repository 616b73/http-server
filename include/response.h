#ifndef RESPONSE_H
#define RESPONSE_H

void send_static_file(int client_fd, const char *file_path);
void send_404(int client_fd, const char *file_path);
void send_403(int client_fd);
void send_405(int client_fd);
void send_echo_response(int client_fd, const char *body, int content_length);

#endif
