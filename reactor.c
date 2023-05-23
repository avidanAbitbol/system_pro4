#include "reactor.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <poll.h>

void* createReactor(ThreadPool* pool) {
    Reactor* reactor = (Reactor*)malloc(sizeof(Reactor));
    if (reactor == NULL) {
        return NULL;
    }

    reactor->fds = NULL;
    reactor->numFds = 0;
    reactor->handlers = NULL;
    reactor->running = 0;
    reactor->pool = pool; // Add this line to assign the thread pool

    return reactor;
}

void stopReactor(void *this) {
    Reactor* reactor = (Reactor*)this;
    if (reactor->running) {
        reactor->running = 0;
        pthread_join(reactor->thread, NULL);
    }

    free(reactor->fds);
    free(reactor->handlers);
    free(reactor);
}

void* reactorThread(void* arg) {
    Reactor* reactor = (Reactor*)arg;

    while (reactor->running) {
        int numReady = poll(reactor->fds, reactor->numFds, -1);
        if (numReady == -1) {
            continue;
        }

        for (int i = 0; i < reactor->numFds; ++i) {
            if (reactor->fds[i].revents & POLLIN) {
                handler_t handler = reactor->handlers[i];
                if (handler != NULL) {
                    handler(reactor->fds[i].fd);
                }
            }
        }
    }

    return NULL;
}

void startReactor(void *this) {
    Reactor* reactor = (Reactor*)this;
    if (reactor->running) {
        return;
    }

    reactor->running = 1;
    pthread_create(&reactor->thread, NULL, reactorThread, reactor);
}

void addFd(void* this, int fd, handler_t handler) {
    Reactor* reactor = (Reactor*)this;

    // Allocate memory for new file descriptor and handler
    struct pollfd* new_fds = (struct pollfd*)realloc(reactor->fds, (reactor->numFds + 1) * sizeof(*new_fds));
    handler_t* new_handlers = (handler_t*)realloc(reactor->handlers, (reactor->numFds + 1) * sizeof(*new_handlers));

    // Check for allocation failures
    if (new_fds == NULL || new_handlers == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        // Handle error condition
        return;
    }

    reactor->fds = new_fds;
    reactor->handlers = new_handlers;

    // Set file descriptor values
    reactor->fds[reactor->numFds].fd = fd;
    reactor->fds[reactor->numFds].events = POLLIN;
    reactor->fds[reactor->numFds].revents = 0;

    // Set handler
    reactor->handlers[reactor->numFds] = handler;

    // Increase number of file descriptors
    reactor->numFds++;
}



void WaitFor(void *this) {
    Reactor* reactor = (Reactor*)this;
    pthread_join(reactor->thread, NULL);
}
