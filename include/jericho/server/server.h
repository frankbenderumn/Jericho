/**
* 5-24-2022: May add a honeypot
* TODO: upgrade to poll instead of select()
* TODO: integrate ssl and extern C dependencies
* TODO: get timeout threads working
* TODO: send right after connect to test
*       client and prevent recv hang
* TODO: integrate postgresql database
* TODO: Iris compiler
* TODO: write unit test library
* TODO: brainstorm load balancers with a cluster (zeromq?)
* TODO: kubernetes? jenkinsCI?
*/

#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include "server/defs.h"
#include "server/event.h"
#include "server/socket.h"
#include "server/frame.h"
#include "server/ping_pong.h"
#include "server/shutdown.h"
#include "server/client.h"
#include "server/router.h"
#include "api/router.h"

extern EventManager event_manager;

void connect(void* targ);
static void* service(void* targ);
void recv_websocket(Any args);
int run(SOCKET* server, Client** clients, SSL_CTX* ctx, ThreadPool* tpool, Router* router);

#endif