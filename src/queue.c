#include "queue.h"

void queue_init(ThreadQueue *q) {
    q->front = 0;
    q->rear = 0;
    q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void queue_push(ThreadQueue *q, int client_fd) {
    pthread_mutex_lock(&q->mutex);

    if (q->count < QUEUE_SIZE) {
        q->client_fds[q->rear] = client_fd;
        q->rear = (q->rear + 1) % QUEUE_SIZE;
        q->count++;
        pthread_cond_signal(&q->cond);
    } else {
        // If the queue is full, we could drop the connection or block.
        // For simplicity, we drop it (should rarely happen).
    }

    pthread_mutex_unlock(&q->mutex);
}

int queue_pop(ThreadQueue *q) {
    pthread_mutex_lock(&q->mutex);

    while (q->count == 0) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }

    int client_fd = q->client_fds[q->front];
    q->front = (q->front + 1) % QUEUE_SIZE;
    q->count--;

    pthread_mutex_unlock(&q->mutex);

    return client_fd;
}

void queue_destroy(ThreadQueue *q) {
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
}
