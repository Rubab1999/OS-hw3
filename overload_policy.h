#ifndef OVERLOAD_POLICY_H
#define OVERLOAD_POLICY_H

// Overload policies
typedef enum {
    BLOCK,        // Block until space becomes available
    DROP_TAIL,    // Drop the incoming request
    DROP_HEAD,    // Drop the oldest request in the queue
    BLOCK_FLUSH,  // Block until the queue is empty, then drop the request - bonus
    DROP_RANDOM   // Drop random requests in the queue - bonus
} OverloadPolicy;

#endif // OVERLOAD_POLICY_H
