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

	DEBUG("Request: %s\n", (char*)frame->request);

	/* Advance our pointers before the first next_byte(). */
	const char* p = strstr((const char *)frame->request, "\r\n\r\n");
	if (p == NULL) { 
        PERR(ESERVER, "An empty line with \\r\\n was expected!\n"); 
        client_set_state(frame->client, SOCKST_NULL); 
    }

	frame->cur_pos = (size_t)((ptrdiff_t)(p - (char *)frame->request)) + 4;
	frame->received = frame->cur_pos;

	DEBUG("FRAME bytes received: %i\n", (int)frame->received);
	DEBUG("FRAME current position: %i\n", (int)frame->cur_pos);

	response = (char*)malloc(sizeof(char) * ACCEPT_LEN);
	if (handshake((char *)frame->request, &response) < 0) {
		DEBUG("Cannot get handshake response, request was: %s\n", frame->request);
		return (-1);
	}

	// printf("Handshaked, response: \n"
	// 	"------------------------------------\n"
	// 	"%s"
	// 	"------------------------------------\n",
	// response);

    // printf("RESPONSE SIZE IS: %i\n", (int)strlen(response) + 1);

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

int run(SOCKET* server, Client** clients, SSL_CTX* ctx, ThreadPool* tpool) {
    while (1) {
        fd_set reads;
        reads = wait_on_clients(*server, clients);

        if (FD_ISSET(*server, &reads)) {
            Client* client = get_client(-1, clients);

			client->socket = accept(*server, (struct sockaddr*) &(client->address), &(client->address_length));

			if (!ISVALIDSOCKET(client->socket)) {
				PFAIL(ECONN, "accept() failed. (%d)\n", SOCKERR());
			}

			char address_buffer[16];
			client_get_address(client, address_buffer);
			CYA("New connection from %s.\n", address_buffer);
			PLOG(LSERVER, "New connection from %s.", address_buffer);

			client->ssl = SSL_new(ctx);
			if (!client->ssl) {
				PFAIL(ECONN, "SSL_new(ctx) failed.");
				fprintf(stderr, "SSL_new() failed.\n");
				return 1;
			}


			SSL_set_fd(client->ssl, client->socket);
			if (SSL_accept(client->ssl) != 1) {
				//SSL_get_error(client->ssl, SSL_accept(...));
				ERR_print_errors_fp(stderr);
				drop_client(client, clients);
			} else {
				printf("SSL connection using %s\n", SSL_get_cipher(client->ssl));
			}
        }

        Client* client = *clients;

        while (client) {
            
			// iterate through clients
            Client* next = client->next;

            if (FD_ISSET(client->socket, &reads)) {

                // max request
                if (MAX_REQUEST_SIZE == client->received) {
                    send_400(client);
                    client = next;
                    continue;
                }

                /** STUB: do send before receive for fault tolerance? */

                // receives bytes from client and asserts against request limit
                int r = SSL_read(client->ssl, client->request + client->received, MAX_REQUEST_SIZE - client->received); 

                if (r > 0) { // bytes received

                    PLOG(LSERVER, "Request received from client: <client-address>");
                    MAG("REQUEST: %s\n", client->request);

                    client->received += r; // increment bytes received
                    client->request[client->received] = 0; 
                    char* q = strstr(client->request, "\r\n\r\n");

                    // if http response aka (contains \r\n\r\n)
                    /** TODO: switch from scan method to strtok_r */
                    if (q) {
                        if (scan("Connection: keep-alive", client->request)) {
                            client_set_state(client, SOCKST_ALIVE);
                            printf("Setting state\n");
                        } else if (scan("Connection: closed", client->request)) {
                            client_set_state(client, SOCKST_CLOSING);
                            printf("Setting state\n");
                        } else if (scan("Connection: Upgrade", client->request)) {
                            client_set_state(client, SOCKST_UPGRADING);
                            printf("Setting state\n");
                        } else {
                            BRED("INVALID HTTP REQUEST DETECTED\n");
                        }
                    }

                    SocketState state;
                    switch(state = client_get_state(client)) {
                        case SOCKST_ALIVE:
                            parser2::parse(client, clients);
                            memset(client->request, 0, strlen(client->request));
                            // drop_client(client, &clients);
                            break;
                        case SOCKST_CLOSING:
                            PLOG(LSERVER, "Dropping client: <client-address>");
                            drop_client(client, clients);
                            break;
                        case SOCKST_UPGRADING: 
                            PLOG(LSERVER, "Upgrading socket fd: %i", client->socket);
                            // PLOG(LSERVER, "Client request: %s", client->request);
                            // connect((void*)client); non threaded version
                            thread_pool_add(tpool, connect, (void*)client); // spawn thread for web socket
                            break;
                        case SOCKST_OPEN_WS:
                            printf("Client request: %s\n", (unsigned char*)client->request);
                            thread_pool_add(tpool, recv_websocket, (void*)client);
                            break;
                        default:
                            PERR(ESERVER, "AN UNEXPECTED STATE WAS ENCOUNTERED!\n");
                            break;
                    }
                }
            }
            client = next;
        }
    }
}