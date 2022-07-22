#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_ 

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include <unistd.h>

#include "server/defs.h"
#include "server/thread_pool.h"

struct ThreadPool;
struct Worker;
struct Dispatcher;
typedef struct ThreadPool ThreadPool;
typedef struct Worker Worker;
typedef struct Dispatcher Dispatcher;
typedef void (*__ThreadFn)(Any arg);

ThreadPool* thread_pool_create(size_t num);
void thread_pool_destroy(ThreadPool* tpool);
bool thread_pool_add(ThreadPool* tpool, __ThreadFn fn, Any arg);
void thread_pool_wait(ThreadPool* tpool);

Worker* worker_create(__ThreadFn fn, Any arg);
void worker_destroy(Worker* worker);
Worker* get_worker(ThreadPool* tpool);

// switch to dispatcher job
void* work(Any arg);

// implementation specific
void job(Any arg);

#ifndef MAX_THREADS
#define MAX_THREADS 5000
#endif

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 10
#endif

typedef enum WorkerType {
    WROLE_NULL,
    WROLE_MAIN,
    WROLE_PRIO_1, 
    WROLE_PRIO_2,
    WROLE_PRIO_3
} WorkerType;

struct Worker {
    __ThreadFn fn;
    Any arg;
    WorkerType role;
    int idx;
    Worker* next;
};

struct ThreadPool {
    Worker* head;
    Worker* tail;
    pthread_mutex_t worker_mutex;
    pthread_cond_t worker_cond;
    pthread_cond_t working_cond;
    Worker pool[MAX_THREADS];
    size_t working_count;
    size_t thread_count;
    bool shutdown;
};

#endif