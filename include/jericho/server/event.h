#ifndef SERVER_EVENT_H_
#define SERVER_EVENT_H_

#include "server/defs.h"
#include "server/thread_pool.h"

// implentation specifc
typedef void (*__Open)(Client* conn);
typedef void (*__Close)(Client* conn);
typedef void (*__Message)(Client* conn, const unsigned char* message, uint64_t size, int type);
typedef void (*__Dispatch)(Client* conn, ThreadPool* tpool, __ThreadFn fn, void* arg);

typedef struct EventManager {
    __Open open;
    __Close close;
    __Message message;
    __Dispatch dispatch;
} EventManager;

#endif