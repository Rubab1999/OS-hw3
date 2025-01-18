#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "queue.h"
#include "overload_policy.h"

extern OverloadPolicy overload_policy;

// Create
Queue* createQueue(int capacity) {
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    queue->capacity = capacity;
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    return queue;
}

// Destroy
void destroyQueue(Queue *queue) {
    Request *current = queue->head;
    while (current != NULL) {
        Request *next = current->next;
        free(current);
        current = next;
    }
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->not_full);
    pthread_cond_destroy(&queue->not_empty);
    free(queue);
}

// Add a new request to the queue
/*void enqueue(Queue *queue, Request *request) {
    pthread_mutex_lock(&queue->lock);
    while (queue->size == queue->capacity) {
        pthread_cond_wait(&queue->not_full, &queue->lock);
    }

    request->next = NULL;
    if (queue->size == 0) {
        queue->head = queue->tail = request;
    } else {
        queue->tail->next = request;
        queue->tail = request;
    }
    queue->size++;
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->lock);
} //  working function */

void enqueue(Queue *queue, Request *request) {
    pthread_mutex_lock(&queue->lock);
    while (queue->size == queue->capacity) {                   // Added to handle overload
        if (overload_policy == BLOCK) {
            pthread_cond_wait(&queue->not_full, &queue->lock); // Block until space becomes available
        }
        else if (overload_policy == DROP_TAIL) {               // Drop the incoming request
				 printf("Overload: Request dropped.\n");
				 pthread_mutex_unlock(&queue->lock);
				 Close(request->connection_fd);
				 free(request);
				 return;
        } 
        else if (overload_policy == DROP_HEAD) {    		   // Remove the oldest request
            Request *old_request = dequeue(queue);
            printf("Overload: Dropping oldest request on fd %d.\n", old_request->connection_fd);
            Close(old_request->connection_fd);
            free(old_request);
        }
    }
    request->next = NULL; // Add request to queue
    if (queue->size == 0) {
        queue->head = queue->tail = request;
    } 
    else {
        queue->tail->next = request;
        queue->tail = request;
    }
    queue->size++;
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->lock);
}



// Pop a request from the queue
Request* dequeue(Queue *queue) {
    pthread_mutex_lock(&queue->lock);
    while (queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->lock);
    }

    Request *request = queue->head;
    queue->head = queue->head->next;
    if (queue->head == NULL) {
        queue->tail = NULL; // Queue is now empty
    }

    queue->size--;
    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->lock);

    return request;
}
