#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <strings.h>
#include "http_parser.h"
#include "router.h"

#define PORT 8080
#define BACKLOG 10

void handle_sigchld(int sig) {
    (void)sig; // suppress unused parameter warning
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

int main()
{
    int listen_fd = -1;
    int client_fd = -1;

    struct sockaddr_in server_addr;


    
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(listen_fd < 0)
    {
        perror("Socket cannot be created");
        goto cleanup;
    }

    memset(&server_addr, 0 , sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        goto cleanup;
    }

    if(listen(listen_fd, BACKLOG) < 0)
    {
        perror("Listen failed");
        goto cleanup;
    }

    printf("Server listening on port %d...\n", PORT);

    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    while(1)
    {
        client_fd = accept(listen_fd, NULL, NULL);
        if(client_fd < 0)
        {
            perror("Failed to establish connection");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(client_fd);
            continue;
        }

        if (pid == 0) { // Child process
            close(listen_fd);

            printf("Client connected (PID %d).\n", getpid());

            HttpRequest *req = parse_http_request(client_fd);
            if (req) {
                handle_request(client_fd, req);
                free_http_request(req);
            } else {
                printf("[PID %d] Failed to read or parse request.\n", getpid());
            }
            
            close(client_fd);
            printf("Client disconnected (PID %d).\n\n", getpid());
            exit(0);
        } else {
            // Parent process
            close(client_fd);
        }
    }

cleanup:
    if(listen_fd >= 0)
    {
        close(listen_fd);
    }

    return 0;
}