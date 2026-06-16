#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

#define QUEUE_SIZE 1024

typedef struct {
    int client_fds[QUEUE_SIZE];
    int front;
    int rear;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} ThreadQueue;

void queue_init(ThreadQueue *q);
void queue_push(ThreadQueue *q, int client_fd);
int queue_pop(ThreadQueue *q);
void queue_destroy(ThreadQueue *q);

#endif
