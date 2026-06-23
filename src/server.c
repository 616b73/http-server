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
#include <strings.h>
#include <pthread.h>
#include "http_parser.h"
#include "router.h"
#include "queue.h"

#define PORT 8080
#define BACKLOG 10
#define THREAD_POOL_SIZE 8

ThreadQueue connection_queue;

void *worker_thread(void *arg) {
    (void)arg;
    while (1) {
        int client_fd = queue_pop(&connection_queue);
        
        printf("[Thread %lu] Handling connection.\n", (unsigned long)pthread_self());

        struct timeval tv;
        tv.tv_sec = 5;  // 5 seconds timeout
        tv.tv_usec = 0;
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        int keep_alive = 0;
        do {
            HttpRequest *req = parse_http_request(client_fd);
            if (req) {
                keep_alive = req->keep_alive;
                handle_request(client_fd, req);
                free_http_request(req);
            } else {
                keep_alive = 0; // Timeout or disconnected
            }
        } while (keep_alive);
        
        close(client_fd);
        printf("[Thread %lu] Connection closed.\n\n", (unsigned long)pthread_self());
    }
    return NULL;
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

    queue_init(&connection_queue);
    
    pthread_t pool[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&pool[i], NULL, worker_thread, NULL);
    }

    while(1)
    {
        client_fd = accept(listen_fd, NULL, NULL);
        if(client_fd < 0)
        {
            perror("Failed to establish connection");
            continue;
        }

        queue_push(&connection_queue, client_fd);
    }

cleanup:
    if(listen_fd >= 0)
    {
        close(listen_fd);
    }

    return 0;
}