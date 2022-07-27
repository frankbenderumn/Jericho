/*
*   5-24-2022: Need to upgrade to wss
*/

#ifndef SERVER_WEBSOCKET_H_
#define SERVER_WEBSOCKET_H_

#include "server/frame.h"
#include "server/socket.h"

/***********************************************************
*   Handshake key
************************************************************/
int handshake_key(char *wsKey, unsigned char **dest);

/***********************************************************
*   Handshake
************************************************************/
int handshake(char *hsrequest, char **hsresponse);

/***********************************************************
*   Upgrade
************************************************************/
int upgrade(Frame* frame);

#endif