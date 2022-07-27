#ifndef SERVER_SHUTDOWN_H_
#define SERVER_SHUTDOWN_H_

#include "server/client.h"
#include "server/frame.h"

/***********************************************************
*   Timeout thread
************************************************************/
static Any close_timeout(Any p);

/***********************************************************
*   Start timeout thread
************************************************************/
int start_close_timeout(Client* conn);

/***********************************************************
*   Signal close and set mutexes
************************************************************/
int do_close(Frame* frame, int close_code);

#endif