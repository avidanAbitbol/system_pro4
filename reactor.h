#ifndef REACTOR_H
#define REACTOR_H

#include <sys/poll.h>
#include <pthread.h>
#include "threadpool.h"

typedef void (*handler_t)(int);

typedef struct {
    struct pollfd* fds;
    int epollFd;
    ThreadPool* pool;
    int numFds;
    handler_t* handlers;
    pthread_t thread;
    int running;
} Reactor;

void* createReactor(ThreadPool* pool);
void stopReactor(void *this);
void startReactor(void *this);
void addFd(void *this, int fd, handler_t handler);
void WaitFor(void *this);

#endif // REACTOR_H
