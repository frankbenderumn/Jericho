/**************************************************************
   Frank Bender
   5-22-2022: May want to test if re-entrant safe
   but only using one signal currently so may be fine 
   for now
**************************************************************/

#ifndef SERVER_CLIENT_H_
#define SERVER_CLIENT_H_

#include "server/defs.h"
#include "server/shutdown.h"
#include "server/socket.h"
#include "server/frame.h"

/***********************************************************
*   Accessors
************************************************************/
void client_get_address(struct Client* ci, char* dest);

void client_get_full_address(struct Client* ci, char* dest, uint16_t* port);

/***********************************************************
*   Atomic Accessors
************************************************************/
SocketState client_get_state(Client* client);

/***********************************************************
*   Atomic Accessors
************************************************************/
void client_set_state(Client* client, SocketState state);

/***********************************************************
*   Atomic Accessors
************************************************************/
void client_set_grade(Client* client, bool grade);

/***********************************************************
*   Atomic Accessors
************************************************************/
bool client_get_grade(Client* client);

/***********************************************************
*   Collection Accessors
************************************************************/
Client* get_client(SOCKET s, Client** clients);

/***********************************************************
*   Atomic Accessors
************************************************************/
void drop_client(Client* client, Client** clients);

/***********************************************************
*   Atomic Accessors
************************************************************/
fd_set wait_on_clients(SOCKET server, Client** clients);

/***********************************************************
*   Shutdown
************************************************************/
int ws_close_client(Client *client);

/***********************************************************
*   Atomic Accessors
************************************************************/
void close_client(Client* client, int lock);


#endif