/*
* Frank Bender: This file needs to be split up
*/
#ifndef SERVER_SOCKET_H_
#define SERVER_SOCKET_H_

#include "server/defs.h"
#include "server/response.h"
#include "server/event.h"
#include "server/web_socket.h"
#include "util/scan.h"

/***********************************************************
*   Atomic Accessors (should not be void) TODO: deperecate
************************************************************/
void socket_create(int port, int reuse);

SOCKET socket_create(const char* host,
                    int port, int reuse, int family, int socktype);

/***********************************************************
*   Atomic Accessors
************************************************************/
ssize_t broadcast(Client* conn, const void *buf, size_t len, int flags);

/***********************************************************
*   Atomic Accessors
************************************************************/
const char* get_content_type(const char* path);

/***********************************************************
*   Atomic Accessors
************************************************************/
int link(Frame* frame);

/***********************************************************
*  extracts Sec-Websocket-Accept key
************************************************************/
int handshake_key(char* key, unsigned char** dest);

/***********************************************************
*   do websocket handshake
************************************************************/
void handshake_response(Client* conn, const char* path, const char* subkey);

/***********************************************************
*   run SHA-1 and base64
************************************************************/
char* socket_key(const char* subkey);

/***********************************************************
*   Moved function (sends content file)
************************************************************/
// void serve_resource(Conn* conn, const char* path);

/***********************************************************
*   Polls for request (TODO) use epoll instead of select?
************************************************************/
void server_get_request(int *conn, char *req_buf, int port);

/***********************************************************
*   Atomic Accessors (DUPLICATE?)
************************************************************/
void socket_close(int fd);

#endif