#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT 8080
#define BACKLOG 10
#define BUFFER_SIZE 1024

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

    char buffer[BUFFER_SIZE];
    
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

            ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE-1);
            if (bytes_read > 0)
            {
                buffer[bytes_read] = '\0';

                char method[16], uri[256], version[16];
                if (sscanf(buffer, "%15s %255s %15s", method, uri, version) == 3)
                {
                    printf("[PID %d] Parsed Request - Method: %s, URI: %s, Version: %s\n", getpid(), method, uri, version);
                }

                char *headers_end = strstr(buffer, "\r\n\r\n");
                if (headers_end) {
                    // Headers received
                }

                // Phase 3: Serve Static Files
                char file_path[512] = "public";
                if (strcmp(uri, "/") == 0) {
                    strncat(file_path, "/index.html", sizeof(file_path) - strlen(file_path) - 1);
                } else {
                    strncat(file_path, uri, sizeof(file_path) - strlen(file_path) - 1);
                }

                int file_fd = open(file_path, O_RDONLY);
                if (file_fd < 0) {
                    const char *not_found_response = 
                        "HTTP/1.1 404 Not Found\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: 13\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "404 Not Found";
                    write(client_fd, not_found_response, strlen(not_found_response));
                    printf("[PID %d] Served 404 Not Found for %s\n", getpid(), file_path);
                } else {
                    struct stat file_stat;
                    fstat(file_fd, &file_stat);
                    off_t file_size = file_stat.st_size;

                    char header[256];
                    snprintf(header, sizeof(header),
                             "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/html\r\n"
                             "Content-Length: %ld\r\n"
                             "Connection: close\r\n"
                             "\r\n", (long)file_size);
                    
                    write(client_fd, header, strlen(header));

                    char file_buffer[1024];
                    ssize_t bytes_read_from_file;
                    while ((bytes_read_from_file = read(file_fd, file_buffer, sizeof(file_buffer))) > 0) {
                        write(client_fd, file_buffer, bytes_read_from_file);
                    }
                    close(file_fd);
                    printf("[PID %d] Served %s (%ld bytes)\n", getpid(), file_path, (long)file_size);
                }
            }
            else if (bytes_read < 0)
            {
                perror("Bytes cannot be read");
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