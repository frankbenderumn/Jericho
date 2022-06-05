#include "server/thread_pool.h"

Worker* worker_create(__ThreadFn fn, Any arg) {
    Worker* worker;
    if (fn == NULL) {
        PERR(ENULL, "Worker not passed a function!");
    }

    worker = (Worker*)malloc(sizeof(Worker));
    worker->fn = fn;
    worker->arg = arg;
    worker->idx = 0;
    worker->role = WROLE_NULL;
    worker->next = NULL;
    return worker;
}

void worker_destroy(Worker* worker) {
    DEBUG("Destroying worker!\n");
    if (worker == NULL) {
        PERR(ENULL, "Worker does not exist!");
    }

    if (worker->idx == -1) {
        PERR(ENULL, "Worker was never dispatched!");
    }

    free(worker);
}

Worker* get_worker(ThreadPool* tpool) {
    Worker* worker;

    if (tpool == NULL) {
        PERR(ENULL, "Thread pool is null!")
        return NULL;
    }

    worker = tpool->head;
    if (worker == NULL) {
        PERR(ENULL, "Worker is null!")
        return NULL;
    }

    if (worker->next == NULL) {
        tpool->head = NULL;
        tpool->tail = NULL;
    } else {
        tpool->head = worker->next;
    }

    return worker;
}

Any work(Any arg) {
    ThreadPool* tpool = (ThreadPool*)arg;
    Worker* worker;

    while (1) {
        pthread_mutex_lock(&(tpool->worker_mutex));
        while(tpool->head == NULL && !tpool->shutdown)
            pthread_cond_wait(&(tpool->worker_cond), &(tpool->worker_mutex));

        if (tpool->shutdown) break;

        worker = get_worker(tpool);
        tpool->working_count++;
        pthread_mutex_unlock(&(tpool->worker_mutex));

        if (worker != NULL) {
            worker->fn(worker->arg);
            worker_destroy(worker);
        }

        pthread_mutex_lock(&(tpool->worker_mutex));
        tpool->working_count--;
        if (!tpool->shutdown && tpool->working_count == 0 && tpool->head == NULL)
            pthread_cond_signal(&(tpool->working_cond));
        pthread_mutex_unlock(&(tpool->worker_mutex));
    }

    tpool->thread_count--;
    pthread_cond_signal(&(tpool->working_cond));
    pthread_mutex_unlock(&(tpool->worker_mutex));
    return NULL;
}

ThreadPool* thread_pool_create(size_t num) {
    ThreadPool* tpool;
    pthread_t thread;
    size_t size;

    if (num == 0) num = 2;

    tpool = (ThreadPool*)calloc(1, sizeof(ThreadPool));
    if (tpool == NULL) {
        PERR(ENEM, "Not enough memory to allocate ThreadPool!");
    }

    tpool->thread_count = num;

    if (pthread_mutex_init(&(tpool->worker_mutex), NULL)) {
        PERR(ENULL, "Failed to initialize work mutex");
    }

    if (pthread_cond_init(&(tpool->worker_cond), NULL)) {
        PERR(ENULL, "Failed to initialize work cond mutex");
    }

    if (pthread_cond_init(&(tpool->working_cond), NULL)) {
        PERR(ENULL, "Failed to initialize working cond mutex");
    }

    tpool->head = NULL;
    tpool->tail = NULL;

    for (int i = 0; i < num; i++) {
        if (pthread_create(&thread, NULL, work, tpool)) {
            PERR(ENULL, "Could not create worker threads!");
        }
        pthread_detach(thread);
    }

    return tpool;
}

void thread_pool_destroy(ThreadPool* tpool) {
    YEL("Destroying thread pool!\n");
    Worker* worker1;
    Worker* worker2;

    if (tpool == NULL) {
        PERR(ENULL, "Thread pool does not exist!");
        return;
    }

    pthread_mutex_lock(&(tpool->worker_mutex));
    worker1 = tpool->head;
    while (worker1 != NULL) {
        worker2 = worker1->next;
        worker_destroy(worker1);
        worker1 = worker2;
    }

    tpool->shutdown = true;
    pthread_cond_broadcast(&(tpool->worker_cond));
    pthread_mutex_unlock(&(tpool->worker_mutex));

    thread_pool_wait(tpool);

    pthread_mutex_destroy(&(tpool->worker_mutex));
    pthread_cond_destroy(&(tpool->worker_cond));
    pthread_cond_destroy(&(tpool->working_cond));

    free(tpool);
}

// dispatcher
bool thread_pool_add(ThreadPool* tpool, __ThreadFn fn, Any arg) {
    DEBUG("Adding worker to thread pool!\n");
    Worker* worker;

    if (tpool == NULL) {
        PERR(ENULL, "Thread Pool is null!");
        return false;
    }

    worker = worker_create(fn, arg);
    if (worker == NULL) {
        PERR(ENULL, "Failed to create worker!");
        return false;
    }

    // dispatch critical section
    pthread_mutex_lock(&(tpool->worker_mutex));
    if (tpool->head == NULL) {
        tpool->head = worker;
        tpool->tail = tpool->head;
    } else {
        tpool->tail->next = worker;
        tpool->tail = worker;
    }

    pthread_cond_broadcast(&(tpool->worker_cond));
    pthread_mutex_unlock(&(tpool->worker_mutex));

    return true;
}

void thread_pool_wait(ThreadPool* tpool) {
    if (tpool == NULL) {
        PERR(ENULL, "Thread pool is null!");
    }

    pthread_mutex_lock(&(tpool->worker_mutex));
    while (1) {
        DEBUG("Thread pool waiting!\n");
        if ((!tpool->shutdown && tpool->working_count != 0) || (tpool->shutdown && tpool->thread_count != 0)) {
            pthread_cond_wait(&(tpool->working_cond), &(tpool->worker_mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(tpool->worker_mutex));
}