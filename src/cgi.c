#include "cgi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void handle_cgi(int client_fd, const char *script_path, HttpRequest *req) {
    int pipe_in[2];
    int pipe_out[2];

    if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Child process
        close(pipe_in[1]);
        close(pipe_out[0]);

        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);

        // Set minimal CGI environment variables
        setenv("REQUEST_METHOD", req->method, 1);
        if (req->content_length > 0) {
            char cl[32];
            snprintf(cl, sizeof(cl), "%d", req->content_length);
            setenv("CONTENT_LENGTH", cl, 1);
        }

        execl(script_path, script_path, NULL);
        
        // If execl fails:
        perror("execl");
        exit(1);
    } else {
        // Parent process
        close(pipe_in[0]);
        close(pipe_out[1]);

        // Send HTTP Response Line
        const char *ok_line = "HTTP/1.1 200 OK\r\n";
        write(client_fd, ok_line, strlen(ok_line));

        // Write body to script's STDIN if it's a POST
        if (strcmp(req->method, "POST") == 0 && req->body && req->content_length > 0) {
            write(pipe_in[1], req->body, req->content_length);
        }
        close(pipe_in[1]); // Close to send EOF to child

        // Read script's output and send to client
        char buffer[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer))) > 0) {
            write(client_fd, buffer, bytes_read);
        }
        close(pipe_out[0]);

        // Wait for child to finish
        waitpid(pid, NULL, 0);
        printf("[PID %d] Executed CGI script %s\n", getpid(), script_path);
    }
}
