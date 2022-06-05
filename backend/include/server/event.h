#ifndef SERVER_EVENT_H_
#define SERVER_EVENT_H_

#include "server/defs.h"
#include "server/thread_pool.h"
#include "server/client.h"
#include "prizm/error.h"

// seperate event concerns to make fully c and avoid picojson dependency in server stack
#include <picojson.h>

// implentation specifc
typedef void (*__Open)(Client* conn);
typedef void (*__Close)(Client* conn);
typedef void (*__Message)(Client* conn, const unsigned char* message, uint64_t size, int type);
typedef void (*__Dispatch)(Client* conn, ThreadPool* tpool, __ThreadFn fn, void* arg);

void open(Client* client);
void dispatch(Client* conn, ThreadPool* tpool, __ThreadFn fn, Any args);
void close(Client* client);
void message(Client* conn, const unsigned char* message, uint64_t size, int type);

typedef struct EventManager {
    __Open open;
    __Close close;
    __Message message;
    __Dispatch dispatch;
} EventManager;

#endif