#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

typedef struct Request {
    int connection_fd; 
    struct timeval arrival;
    struct Request *next;
} Request;

typedef struct Queue {
    Request *head;           
    Request *tail;           
    int size;               	// request number
    int capacity;           	// max requests number
    pthread_mutex_t lock;    
    pthread_cond_t not_full;	// queue not full
    pthread_cond_t not_empty;	// queue not empty
} Queue;

Queue* createQueue(int capacity); 
void destroyQueue(Queue *queue); 
void enqueue(Queue *queue, Request *request);
Request* dequeue(Queue *queue);

#endif // QUEUE_H
