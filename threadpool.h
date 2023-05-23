// threadpool.h

#ifndef THREADPOOL_H
#define THREADPOOL_H

typedef struct ThreadPool ThreadPool;

typedef void (*ThreadPoolWorkFunction)(void*);

ThreadPool* threadpool_create();
void threadpool_destroy(ThreadPool* pool);
void threadpool_add_work(ThreadPool *pool, ThreadPoolWorkFunction function, void *args);

#endif // THREADPOOL_H
