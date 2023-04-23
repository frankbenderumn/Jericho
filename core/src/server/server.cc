#include <openssl/ssl.h>

#include "system/system.h"
#include "server/server.h"
#include "server/request.h"
#include "server/fetch.h"
#include "celerity/celerity.h"
#include "message/message_buffer.h"
#include "message/message_broker.h"
#include "message/bifrost.h"
#include "api/api_helper.h"
#include "message/callback.h"

using namespace Jericho;

int connection_setup(System* router, Client** clients, SSL_CTX* ctx, SOCKET* server) {
	Client* client = get_client(-1, clients, &router->num_clients);

	// client count debug
	// BBLU("Number of clients: %li\n", router->num_clients);
	// std::string client_ct_write = "Number of clients: " + std::to_string(router->num_clients);
	// JFS::write("./log/clients", client_ct_write.c_str());

	client->socket = accept(*server, (struct sockaddr*) &(client->address), &(client->address_length));

	if (!ISVALIDSOCKET(client->socket)) {
		PFAIL(ECONN, "accept() failed. (%d)\n", SOCKERR());
	}

	char address_buffer[16];
	client_get_address(client, address_buffer);
	// CYA("0New connection from %s.\n", address_buffer);
	PLOG(LSERVER, "New connection from %s.", address_buffer);

	// char address_buffer2[16];
	// uint16_t p2;
	// client_get_full_address(client, address_buffer2, &p2);
	// BYEL("New connection from %s:%i.\n", address_buffer, p2);

    // Print the options set in the SSL context
    unsigned long options = SSL_CTX_get_options(ctx);
    printf("SSL context options: %lu\n", options);

	if (options & SSL_OP_NO_TLSv1_3) {
		printf("SSL_OP_NO_TLSv1_3 is set\n");
	} else {
		printf("SSL_OP_NO_TLSv1_3 is not set\n");
	}

	// // Get the SSL context options as a string
    // const char *options_str = SSL_CTX_get_options_string(ctx);

    // // Print the SSL context options string
    // printf("SSL context options: %s\n", options_str);


	if (ctx == NULL) {
		BRED("CTX FUCKED UP!\n");
	}

	client->ssl = SSL_new(ctx);

	if (SSL_new(ctx) == NULL) {
		BRED("YALL DONE FUCKED UP!\n");
	}

	if (!client->ssl) {
		// PFAIL(ECONN, "SSL_new(ctx) failed.");
		fprintf(stderr, "SSL_new() failed.\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	SSL_set_fd(client->ssl, client->socket);
	if (SSL_accept(client->ssl) != 1) {
		// SSL_get_error(client->ssl, SSL_accept(...));
		ERR_print_errors_fp(stderr);
		BRED("OAHAXCA\n");
		drop_client(client, clients, &router->num_clients); // this will cause bugs on mac localhost test
	} else {
		// printf("SSL connection using %s\n", SSL_get_cipher(client->ssl));
	}
	return 0;
}

int run(SOCKET* server, Client** clients, SSL_CTX* ctx, ThreadPool* tpool, System* router) {

    while (1) {
        fd_set reads;

		// router->bifrost()->burst(router);

		BLU("Awaiting clients...\n");

        reads = wait_on_clients(*server, clients);

        if (FD_ISSET(*server, &reads)) {
			if (connection_setup(router, clients, ctx, server) < 0) {
				BRED("Server::connection_setup: Problem connecting to client!\n");
			}
        }

        Client* client = *clients;

        while (client) {

			// if (router->bifrost()->poll(router, client->url, client, clients)) {
			// 	break;
			// }
            
			// iterate through clients
            Client* next = client->next;

            if (FD_ISSET(client->socket, &reads)) {

				router->bifrost()->dumpBrokerSizes();

				// Benchmark* bm = bm_start("serve");
				// if (router->federator()->local()->resource() > 0) {
				// 	sleep(router->federator()->local()->resource());
				// }

                // max request
                if (MAX_REQUEST_SIZE == client->received) {
                    send_400(client);
                    client = next;
					BYEL("Server::main: Continuing: client received to large. Greate than MAX_REQUEST_SIZE!\n"); 
                    continue;
                }

				Request* req = new Request(client, MAX_REQUEST_SIZE);
				int r = req->client->received;

                if (r > 0) { // bytes received
                    MAG("REQUEST\n=====================================\n"); 
					MAG("%s\n", client->request);

					req->eval();

					if (req->protocol == "JOB") {
						std::string response;
						int status = router->bifrost()->fulfill(response, req, client, clients);
						if (status == 1) {
							picojson::object o;
							o["host"] = picojson::value(req->header("Host"));
							o["message"] = picojson::value(response);
							std::string s = JsonResponse::ws(200, "placeholder", "null", &o);
							BMAG("Serialized ws: %s\n", s.c_str());
							router->ws_send(s.c_str());
							drop_client(client, clients, &router->num_clients);
							break;
						} else if (status == 0) {
							drop_client(client, clients, &router->num_clients);
							break;
						} else {
							BGRE("\n\nRicochet in progress!\n\n");
						}
					}

					if (!req->valid) {
						delete req;
						drop_client(client, clients, &router->num_clients);
						break;
					}

					client->url = req->headers["Host"];
					req->args["content"] = req->content; // need to change this feature in the API Macro, make api rely on request
					// client->received += r; // increment bytes received
                    // client->request[client->received] = 0; 
                    // char* q = strstr(client->request, "\r\n\r\n");

					std::string conn = req->headers["Connection"];
					if (conn == "keep-alive") {
						client_set_state(client, SOCKST_ALIVE);
					} else if (conn == "closed") {
						client_set_state(client, SOCKST_CLOSING);
					} else if (conn == "Upgrade") {
						client_set_state(client, SOCKST_UPGRADING);
					} else {
						BRED("Server::run: Invalid Http request detected!\n");
					}

                    SocketState state;
                    switch(state = client_get_state(client)) {
                        case SOCKST_ALIVE:
							BRED("Getting served!\n");
							router->bifrost()->serve(router, client, clients, req);
                            router->bifrost()->dumpBrokerSizes();
							break;
                        case SOCKST_CLOSING:
                            break;
                        case SOCKST_UPGRADING: 
                            PLOG(LSERVER, "Upgrading socket fd: %i", client->socket);
							router->ws(client);
							client->websocket = true;
                            thread_pool_add(tpool, connect, (void*)client); // spawn thread for web socket
                            break;
                        case SOCKST_OPEN_WS:
							client->websocket = true;
                            thread_pool_add(tpool, recv_websocket, (void*)client);
                            break;
                        default:
                            PERR(ESERVER, "AN UNEXPECTED STATE WAS ENCOUNTERED!\n");
                            break;
                    }
                } else {
					BRED("Server::run: No bytes received!\n");
				}

				/**************
				 * CLEANUP
				**************/
				if (!client->websocket) {
					delete req;
					drop_client(client, clients, &router->num_clients);
				}
				// } else if (!req->async) {
				// 	delete req;
				// }

            }
            client = next;
        }
    }
}





// old code for websockets, need to revamp/clean up
// =============================================================================

int link(Frame* frame, Client* client) {
    BCYA("Linking...\n");
	char* response; /* Handshake response message. */

    // ssize_t n;
	// if (n = recv(frame->client->socket, frame->request, sizeof(frame->request) - 1, 0) < 0) {
    //     PERR(ESERVER, "No data recieved in handshake\n");
    //     // client_set_state(frame->client, SOCKST_NULL); 
	// 	return -1;
    // } 

    char tbuffer[500000];
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
	frame.request[WS_PACKET] = (unsigned char)'\0';
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

void ws_to_client(void* targ) {
	Frame frame;
	int close_timeout;
	Client* client = (Client*)targ;

	memset(&frame, 0, sizeof(frame));
	frame.client = client;

	memcpy(frame.request, (unsigned char*)client->request, sizeof(frame.request));
	frame.request[WS_PACKET] = (unsigned char)'\0';
	frame.received = client->received;

	printf("Frame client fd: %i\n", frame.client->socket);

	char buffer[WS_PACKET];
	memcpy(buffer, frame.request, sizeof(buffer));
	buffer[WS_PACKET] = 0;

	ws_sendframe_txt(frame.client, buffer);
	memset(client->request, 0, sizeof(client->request));
}

void connect(void* targ) {
    Frame frame; // WebSocket frame data
	int close_timeout; // Time-out close thread
	Client* client = (Client*)targ;
	
	// Prepare frame data
	memset(&frame, 0, sizeof(frame));
	frame.client = client;

	memcpy(frame.request, (unsigned char*)client->request, sizeof(frame.request));
	frame.request[WS_PACKET] = (unsigned char)'\0';
	frame.received = client->received;

	printf("Frame client fd: %i\n", frame.client->socket);
	

	printf("Connecting client...\n");
	if (link(&frame, client) < 0) {
		PERR(ECONN, "Failed to establish connection!");
		// goto closed;
	}


    // TODO: Implement message passing over socket
	ws_sendframe_txt(frame.client, "Web sockets enabled!");
	frame.client->received = 0;
	memset(frame.client->request, 0, sizeof(client->request));
	BGRE("STARTING WEBSOCKET!\n");

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