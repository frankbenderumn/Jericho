#include "server/server.h"

int link(Frame* frame, Client* client) {
    BCYA("Linking...\n");
	char* response; /* Handshake response message. */

    // ssize_t n;
	// if (n = recv(frame->client->socket, frame->request, sizeof(frame->request) - 1, 0) < 0) {
    //     PERR(ESERVER, "No data recieved in handshake\n");
    //     // client_set_state(frame->client, SOCKST_NULL); 
	// 	return -1;
    // } 

    char tbuffer[5000];
    sprintf(tbuffer, "Client request is: %s\n", frame->request);
    write_thread(frame->client->socket, tbuffer);

	BYEL("Request: %s\n", (char*)frame->request);

	/* Advance our pointers before the first next_byte(). */
	const char* p = strstr((const char *)frame->request, "\r\n\r\n");
	if (p == NULL) { 
        PERR(ESERVER, "An empty line with \\r\\n was expected!\n"); 
        client_set_state(frame->client, SOCKST_NULL); 
    }

	frame->cur_pos = (size_t)((ptrdiff_t)(p - (char *)frame->request)) + 4;
	frame->received = frame->cur_pos;

	BYEL("FRAME bytes received: %i\n", (int)frame->received);
	BYEL("FRAME current position: %i\n", (int)frame->cur_pos);

	response = (char*)malloc(sizeof(char) * ACCEPT_LEN);
	if (handshake((char *)frame->request, &response) < 0) {
		DEBUG("Cannot get handshake response, request was: %s\n", frame->request);
		return (-1);
	}

	printf("Handshaked, response: \n"
		"------------------------------------\n"
		"%s"
		"------------------------------------\n",
	response);

    // strcat(response, "sUBSUSusush");

    printf("RESPONSE SIZE IS: %i\n", (int)strlen(response) + 1);

	/* Send handshake. */
	if (broadcast(frame->client, response, strlen(response), 0) < 0) {
		free(response);
		PERR(ESERVER, "As error has occurred while handshaking!\n");
		return -1;
	}

    // do_pong(frame, strlen(frame->payload), FRAM)

	// Trigger events and clean up buffers
	event_manager.open(frame->client);
	free(response);
    return 0;
}

void recv_websocket(Any arg) {
	Frame frame; // WebSocket frame data
	int close_timeout; // Time-out close thread
	Client* client = (Client*)arg;
	
	// Prepare frame data
	memset(&frame, 0, sizeof(frame));
	frame.client = client;

	memcpy(frame.request, (unsigned char*)client->request, sizeof(frame.request));
	frame.request[2048] = (unsigned char)'\0';
	frame.received = client->received;
	/* Read next frame until client disclientects or an error occur. */
	// while (client_get_state(frame.client) == SOCKET_OPEN_WS) {
	while (next_frame(&frame) >= 0 && client_get_state(frame.client) == SOCKST_OPEN_WS) {
	// 	/* Text/binary event. */
		if ((frame.type == FRAME_TXT || frame.type == FRAME_BIN) && !frame.error) {
			BGRE("Receiving Message\n");
			event_manager.message(frame.client, frame.message, frame.size, frame.type);
		}
		/* Close event. */
		else if (frame.type == FRAME_CLOSE && !frame.error) {
			/*
			 * We only send a CLOSE frame once, if we're already
			 * in CLOSING state, there is no need to send.
			 */
			if (client_get_state(client) != SOCKST_CLOSING) {
				client_set_state(client, SOCKST_CLOSING);
				BYEL("CLOSE FRAME RECEIVED!\n");
				/* We only send a close frameSend close frame */
				return;
				// do_close(&frame, -1);
			}
			free(frame.message);
			break;			
        }
		// free(frame.message);
		BMAG("LEN OF REQUEST: %i\n", (int)strlen(frame.client->request));
		memset(frame.client->request, 0, sizeof(client->request));
		frame.client->received = 0;
	}
}

void connect(void* targ) {
    Frame frame; // WebSocket frame data
	int close_timeout; // Time-out close thread
	Client* client = (Client*)targ;
	
	// Prepare frame data
	memset(&frame, 0, sizeof(frame));
	frame.client = client;

	memcpy(frame.request, (unsigned char*)client->request, sizeof(frame.request));
	frame.request[2048] = (unsigned char)'\0';
	frame.received = client->received;

	printf("Frame client fd: %i\n", frame.client->socket);
	

	DEBUG("Connecting client...\n");

	if (link(&frame, client) < 0) {
		PERR(ECONN, "Failed to establish connection!");
		// goto closed;
	}


    // TODO: Implement message passing over socket
	ws_sendframe_txt(frame.client, "Web sockets enabled!");
	memset(frame.client->request, 0, sizeof(client->request));
	frame.client->received = 0;
	BMAG("STARTING WEBSOCKET!\n");
	/*
	 * on_close events always occur, whether for client closure
	 * or server closure, as the server is expected to
	 * always know when the client disclientects.
	 */
	// event_manager.close(client);

// TODO: Properly shutdown threads
// closed:
	// close_timeout = client->close_thread;

	/* Wait for timeout thread if necessary. */
	// if (close_timeout) {
	// 	pthread_cond_signal(&client->state_close_cond);
	// 	pthread_join(client->thread_tout, NULL);
	// }

	/* Close clientection properly. */
	// if (client_get_state(client) != SOCKST_CLOSED)
	// 	close_client(client, 1);
	// BRED("Shutting down client!\n");
	// return targ;
	// exit(1);
    return;
}

// static void* service(void* targ) {
//     struct sockaddr_in client; /* Client.                */
//     pthread_t client_thread;   /* Client thread.         */
//     struct timeval time;       /* Client socket timeout. */
//     SOCKET threaded_socket;    /* New opened clientection. */
//     int i;                     /* Loop index.            */

// 	// SOCKET socket = *(int*)targ;
//     SOCKET socket = *(SOCKET*)targ;

// 	int len = sizeof(struct sockaddr_in);
//     printf("Socket initialized\n");

// 	while (1) {
// 		/* Accept. */
// 		threaded_socket = accept(socket, (struct sockaddr *)&client, (socklen_t *)&len);
//         // SOCKET socket_thread = accept(server, (struct sockaddr*) &(client->address), &(client->address_length));

//         printf("Threaded sockets initialized\n");

//         if (timeout) {
//             time.tv_sec = timeout / 1000;
//             time.tv_usec = (timeout % 1000) * 1000;

//             setsockopt(threaded_socket, SOL_SOCKET, SO_SNDTIMEO, &time, sizeof(struct timeval));

//             printf("Setting socket timeout\n");
//         }

//         if (!ISVALIDSOCKET(threaded_socket)) {
//             fprintf(stderr, "accept() failed. (%d)\n", SOCKERR());
//             // return -1;
//             PFAIL(ESERVER, "Failed to accept threaded socket\n");
//             // exit(1);
//         }

// 		/* Adds client socket to socks list. */
// 		pthread_mutex_lock(&global_mutex);
// 		for (i = 0; i < THREAD_POOL; i++) {
// 			if (clientections[i].socket == -1) {
// 				clientections[i].socket = threaded_socket;
// 				clientections[i].state = SOCKST_CONNECTING;
// 				clientections[i].close_thread = FALSE;
// 				clientections[i].last_pong_id = -1;
// 				clientections[i].current_ping_id = -1;
// 				client_set_address(&clientections[i]);

//                 write_thread(i, "Intitializing clientection\n");

// 				if (pthread_mutex_init(&clientections[i].state_mutex, NULL)) {
// 					PERR(ESERVER, "Error on allocating close mutex");
// 				}
// 				if (pthread_cond_init(&clientections[i].state_close_cond, NULL)) {
// 					PERR(ESERVER, "Error on allocating condition var");
// 				}
// 				if (pthread_mutex_init(&clientections[i].send_mutex, NULL)) {
// 					PERR(ESERVER, "Error on allocating send mutex");
// 				}
// 				if (pthread_mutex_init(&clientections[i].ping_mutex, NULL)) {
// 					PERR(ESERVER, "Error on allocating ping/pong mutex");
// 				}
// 				break;
// 			} else {
// 				PWARN(ESERVER, "Socket = -1");
// 			}
// 		}
// 		pthread_mutex_unlock(&global_mutex);

//         printf("Thread clientections initialized\n");

// 		/* Client socket added to socks list ? */
// 		// if (i != THREAD_POOL) {
// 		// 	if (pthread_create(&client_thread, NULL, clientect, &clientections[i])) {
//         //         PERR(ESERVER, "Could not create thread!");
//         //     }
// 		// 	pthread_detach(client_thread);
// 		// } else {
// 		// 	// close_socket(threaded_socket);
//     	// }
//     }
	// free(targ);
    // return;
	// return (targ);
// }