#include <stdio.h>
#include <stdlib.h>
// #include <string.h>
// #include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>



int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_fd < 0)
    {
        perror("Socket cannot be created");
        exit(EXIT_FAILURE);
    }

    printf("Socket created with FD = %d\n", server_fd);

    close(server_fd);

    return 0;
}