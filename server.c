#include "segel.h"
#include "request.h"
#include "queue.h"
#include <pthread.h>
#include "overload_policy.h"

OverloadPolicy overload_policy = BLOCK; // Default overload policy = BLOCK

// Global variables
Queue *request_queue;
int num_threads;
pthread_t *worker_threads;
threads_stats *stats;

void *worker_thread_function(void *arg) {
    int thread_id = *(int *)arg;
    free(arg);
    while (1) {
        Request *req = dequeue(request_queue);

        struct timeval dispatch;
        gettimeofday(&dispatch, NULL);
        stats[thread_id]->total_req++;
        requestHandle(req->connection_fd, req->arrival, dispatch, stats[thread_id]);
        Close(req->connection_fd);
        free(req);
    }
    return NULL;
}
/*
void getargs(int *port, int *num_threads, int *queue_size, int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <port> <num_threads> <queue_size>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *num_threads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
} // this is working code */ 

void getargs(int *port, int *num_threads, int *queue_size, char **schedalg, int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <port> <num_threads> <queue_size> <schedalg>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *num_threads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    *schedalg = argv[4];

    // Determine the overload policy based on the input
    if (strcmp(*schedalg, "block") == 0) {
        overload_policy = BLOCK;
    } 
    else if (strcmp(*schedalg, "dt") == 0) { 
        overload_policy = DROP_TAIL;
    } 
    else if (strcmp(*schedalg, "dh") == 0) { 
        overload_policy = DROP_HEAD;
    } 
    else if (strcmp(*schedalg, "bf") == 0) {     // Bonus
        overload_policy = BLOCK_FLUSH;
    } 
    else if (strcmp(*schedalg, "random") == 0) { // Bonus
        overload_policy = DROP_RANDOM;
    }
    else {
        fprintf(stderr, "Error: Unknown scheduling algorithm '%s'\n", *schedalg);
        exit(1);
    }
}


int main(int argc, char *argv[]) {
    int listenfd, port, clientlen, queue_size;
    struct sockaddr_in clientaddr;
    char *schedalg;
	getargs(&port, &num_threads, &queue_size, &schedalg, argc, argv);
    //getargs(&port, &num_threads, &queue_size, argc, argv);
    request_queue = createQueue(queue_size);
    stats = malloc(num_threads * sizeof(struct Threads_stats *));
    for (int i = 0; i < num_threads; i++) {
        stats[i] = malloc(sizeof(struct Threads_stats));
        stats[i]->id = i;
        stats[i]->stat_req = 0;
        stats[i]->dynm_req = 0;
        stats[i]->total_req = 0;
    }

    worker_threads = malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; i++) {
        int *thread_id = malloc(sizeof(int));
        *thread_id = i;
        pthread_create(&worker_threads[i], NULL, worker_thread_function, thread_id);
    }

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        int connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);

        struct timeval arrival;
        gettimeofday(&arrival, NULL);

        Request *new_request = malloc(sizeof(Request));
        new_request->connection_fd = connfd;
        new_request->arrival = arrival;

        enqueue(request_queue, new_request);
    }

    return 0;
}
