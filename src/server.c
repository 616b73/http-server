#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 8080
#define BACKLOG 10
#define BUFFER_SIZE 1024

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

    client_fd = accept(listen_fd, NULL, NULL);
    if(client_fd < 0)
    {
        perror("Failed to establish connection");
        goto cleanup;
    }

    printf("Client connected. \n");

    ssize_t bytes_read;
    while((bytes_read = read(client_fd, buffer, BUFFER_SIZE-1)) > 0)
    {
        buffer[bytes_read] = '\0';
        printf("Received %zd bytes:\n%s\n", bytes_read, buffer);
    }

    if(bytes_read == 0)
    {
        printf("Client disconnected.\n");
    }
    else if (bytes_read < 0)
    {
        perror("Bytes cannot be read");
    }
    
    cleanup:
      if(client_fd >= 0)
      {
        close(client_fd);
      }
      if(listen_fd >= 0)
      {
        close(listen_fd);
      }

    return 0;
}