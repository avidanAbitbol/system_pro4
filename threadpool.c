#include "threadpool.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h> // Add this for sysconf
#include <stdio.h>  // Add this for fprintf and stderr
typedef struct {
    void (*function)(void*);
    void *args;
} Task;

typedef struct ThreadPool {
    pthread_t* threads;
    int num_threads;
    Task* task_queue;
    int task_queue_size;
    int queue_capacity;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_cond_t empty;
    bool stop;
} ThreadPool;

void* worker(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;
    while (1) {
        pthread_mutex_lock(&(pool->lock));

        while (pool->task_queue_size == 0 && !pool->stop) {
            pthread_cond_wait(&(pool->cond), &(pool->lock));
        }

        if(pool->stop) {
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
        }

        // Get the next task from the queue
        Task task = pool->task_queue[--pool->task_queue_size];

        pthread_mutex_unlock(&(pool->lock));

        // Execute the task function
        (*(task.function))(task.args);
    }

    return NULL;
}

ThreadPool* threadpool_create() {
    ThreadPool* pool = malloc(sizeof(ThreadPool));
    if (pool == NULL) {
        return NULL;
    }

    pthread_mutex_init(&(pool->lock), NULL); // Initialize the mutex

    // Determine the number of available CPU cores
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores <= 0) {
        fprintf(stderr, "Failed to determine the number of CPU cores\n");
        free(pool);
        return NULL;
    }

    pool->num_threads = num_cores;
    pool->task_queue_size = 0;
    pool->queue_capacity = num_cores; // Initial size could be equal to the number of threads.
    pool->task_queue = malloc(pool->queue_capacity * sizeof(Task));
    if (pool->task_queue == NULL) {
        fprintf(stderr, "Failed to allocate memory for the task queue\n");
        free(pool);
        return NULL;
    }
    pool->stop = false;
    pthread_cond_init(&(pool->cond), NULL);
    pthread_cond_init(&(pool->empty), NULL);

    pool->threads = malloc(pool->num_threads * sizeof(pthread_t));
    if (pool->threads == NULL) {
        fprintf(stderr, "Failed to allocate memory for the threads\n");
        free(pool->task_queue);
        free(pool);
        return NULL;
    }

    for (int i = 0; i < pool->num_threads; ++i) {
        pthread_create(&(pool->threads[i]), NULL, worker, pool);
    }

    return pool;
}


void threadpool_destroy(ThreadPool* pool) {
    if (pool == NULL) {
        return;
    }

    pthread_mutex_lock(&(pool->lock));
    pool->stop = true;
    pthread_cond_broadcast(&(pool->cond));
    pthread_mutex_unlock(&(pool->lock));

    for(int i = 0; i < pool->num_threads; i++){
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->task_queue);
    free(pool->threads);
    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->cond));
    pthread_cond_destroy(&(pool->empty));
    free(pool);
}

void threadpool_add_work(ThreadPool* pool, void (*function)(void*), void *args) {
    pthread_mutex_lock(&(pool->lock));

    if(pool->task_queue_size == pool->queue_capacity) {
        // Resizing the queue if needed
        pool->queue_capacity *= 2;
        pool->task_queue = realloc(pool->task_queue, pool->queue_capacity * sizeof(Task));
    }

    // Add the new task to the queue
    pool->task_queue[pool->task_queue_size].function = function;
    pool->task_queue[pool->task_queue_size].args = args;
    ++pool->task_queue_size;

    pthread_cond_signal(&(pool->cond));
    pthread_mutex_unlock(&(pool->lock));
}
