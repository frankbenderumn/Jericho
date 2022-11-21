#include "router/router.h"
#include "server/server.h"
#include "server/request.h"
#include "server/fetch.h"
#include "celerity/celerity.h"
#include "message/message_buffer.h"
#include "message/message_broker.h"
#include "api/api_helper.h"
#include "message/callback.h"

using namespace Jericho;

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

bool authenticate(std::string path, std::string content) {
	std::vector<std::string> args = tokenize(content, '&');
	std::unordered_map<std::string, std::string> kvs;
	for (auto arg : args) {
		std::vector<std::string> kv = tokenize(arg, '=');
		kvs[kv[0]] = kv[1];
	}
	if (containsKey(kvs, std::string("username"))) {
		if (kvs["username"] == "joey") return true;
	} else {
		return false;
	}

	if (containsKey(kvs, std::string("password"))) {
		if (kvs["password"] == "pass1234") return true;
	} else {
		return false;
	}

	return false;
}

int run(SOCKET* server, Client** clients, SSL_CTX* ctx, ThreadPool* tpool, Router* router) {
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

			char address_buffer2[16];
			uint16_t p2;
			client_get_full_address(client, address_buffer2, &p2);
			BYEL("New connection from %s:%i.\n", address_buffer, p2);

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
				// drop_client(client, clients); // this will cause bugs on mac localhost test
			} else {
				printf("SSL connection using %s\n", SSL_get_cipher(client->ssl));
			}
        }

        Client* client = *clients;

		// for server (both server and client are fully autonomous P2P)
		if (router->needsAggregate()) {
			// long long t = std::time(NULL) - router->federator()->startTime() - router->federator()->waitTime();
			// BWHI("TIME TILL TIMEOUT: %lld\n", t * -1);
			if (std::time(NULL) - router->federator()->startTime() >= router->federator()->waitTime()) {
				BRED("FEDERATOR TIME LIMIT REACHED FOR AGGREGATION\n");
				router->cluster()->index()->quorumDrop();
				router->shutdownFederator();
			}

			if (router->federator() != nullptr) {
				std::vector<ClusterQuorum*> newClients = router->cluster()->index()->selectType(FL_JOINED);
				std::vector<ClusterQuorum*> joinedClients = router->cluster()->index()->selectType(FL_DELIVERED);
				std::vector<ClusterQuorum*> dormantClients = router->cluster()->index()->selectType(FL_DORMANT);
				std::vector<ClusterQuorum*> droppedClients = router->cluster()->index()->selectType(FL_DROPPED);
				std::vector<ClusterQuorum*> trainingClients = router->cluster()->index()->selectType(FL_TRAINING);
				BYEL("NEEDED: %i, JOINED: %li, TRAINING: %li, DELIVERED: %li, DORMANT: %li, DROPPED: %li\n", router->federator()->clients(), newClients.size(), trainingClients.size(), joinedClients.size(), dormantClients.size(), droppedClients.size());
				std::vector<std::pair<std::string, std::string>> set;
				for (auto p : newClients) {
					set.push_back({p->host, p->port});
				}

				if (set.size() > 0) {
					if (router->federator()->bytes() != "undefined") {
						router->cluster()->boss()->broadcastNaive(router, client, set, "/train", group_callback, "binary", router->federator()->bytes());
					}
				}

				if (router->cluster()->index()->quorumMet(router->federator()->clients())) {
					BGRE("QUORUM MET, START AGGREGATION\n");
					std::vector<std::string> selection = router->cluster()->index()->quorumSelect(router->federator()->clients());
					if (selection.size() != 0) {
						std::string ports = "[";
						for (auto s : selection) {
							ports += s + ",";
						}
						ports.pop_back(); ports += "]";
						BBLU(" PORTS: %s\n", ports.c_str());
						std::string command = "python3 ./py/fusion.py " + ports + " " + std::to_string(router->federator()->round()) + " " + std::to_string(router->federator()->id());
						std::string results = pipe(command);
						BWHI("AND?\n");
						router->cluster()->index()->quorumUpdate(selection);
						router->federator()->finishRound();
						if (router->federator()->active()) {
							std::string path = "./public/aggregator/aggregate-" + std::to_string(router->federator()->round() - 1) + ".pt"; 
							std::string bytes = Jericho::FileSystem::readBinary(path.c_str());
							std::vector<std::string> nextSelection = router->cluster()->index()->quorumSelect(router->federator()->clients());
							std::vector<std::pair<std::string, std::string>> set;
							for (auto p : nextSelection) {
								set.push_back({"127.0.0.1", p});
							}
							router->federator()->bytes(bytes);
						}
					} else {
						BYEL("STOPPING FEDERATION DUE TO ERROR\n");
						router->shutdownFederator();
					}
				}
			}
		}

		// for client (both server and client are fully autonomous P2P)
		while (router->needsTrain()) {
			if (router->needsTrain()) {
				BMAG("NEED TRAINING\n");
				std::string dir = "./public/cluster/" + router->cluster()->boss()->port();
				std::string command_path = "python3 ./py/torch_load.py " + dir;
				std::string results = pipe(command_path);
				router->train(false);
				if (client == NULL || client == nullptr) {
					BYEL("NEED TO GENERATE A CLIENT\n");
				}
				std::string wts = dir + "/mnist_train.wt";
				std::string bytes = Jericho::FileSystem::readBinary(wts.c_str());
				router->cluster()->boss()->send2(router, client, "127.0.0.1:8080/join-weights", "binary", bytes);
			}
		}

        while (client) {

			MessageBroker* broker = router->cluster()->boss()->poll(client);
			if (broker != NULL) {
				BGRE("FIRING ASYNC CALL\n");
				std::string response = broker->callback()(router, client, broker->response(client));
				BRED("YABBA DABBA DOO\n");
				if (broker->epoch() == 0) {
					if (isHTTP(response)) {
						resource::serve_raw(client, clients, response.c_str());
					} else {
						resource::serve_http(client, clients, response.c_str());
					}
					client->promised = false;
					BBLU("I DONT UNDERSTAND\n");
					drop_client(client, clients); // this segfaults buffer size of 500000 but not 4095 (fixed)
					break;
				}
			}
            
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
				int bytes_received = 0;
				long bytes_needed = 0;
				bool all_read = false;
				bool headers_found = false;
				int r;
                // receives bytes from client and asserts against request limit
				while (!all_read) { 
                	r = SSL_read(client->ssl, client->request + client->received, MAX_REQUEST_SIZE - client->received); 
					bytes_received += r;
					client->received += r; // increment bytes received
					if (r < 5000) {
						all_read = true;
					} else {
						if (!headers_found) {
							char* p = strstr(client->request, "\r\n\r\n");
							if (p != NULL) {
								size_t len = p - &client->request[0];
								BYEL("HEADERS LEN IS: %li\n", len + 4);
								char buf[len + 5];
								strncpy(buf, client->request, len + 4);
								buf[len + 5] = 0;
								char view[200];
								strncpy(view, client->request, 199);
								view[200] = 0;
								BMAG("BUF IS: %s\n", buf);
								BMAG("VIEW IS: %s\n", view);
								char* p2 = strstr(buf, "Content-Length: ");
								if (p2 != NULL) {
									p2 = p2 + strlen("Content-Length: ");
									char* lenEnd = strstr(p2, "\r\n");
									if (lenEnd == NULL) { BRED("\\r\\n not found\n"); }
									size_t sz = lenEnd - p2;
									BBLU("Content length has %li digits\n", sz);
									char clen[sz + 1];
									BBLU("5\n");
									strncpy(clen, p2, sz);
									clen[sz + 1] = 0;
									BBLU("LEN IS: %s\n", clen);
									char* pEnd;
									long length = strtol(clen, &pEnd, 10);
									BBLU("LEN IS: %li\n", length);
									long total = length + len;
									BBLU("BYTE TOTAL SHOULD BE: %li\n", total);
									headers_found = true;
									bytes_needed = total;
								} else {
									BRED("Content-Length not specified\n");
								}						
							} else {
								BRED("Not an HTTP Protocol (\\r\\n\\r\\n not found)\n");
							}
						}
						if ((long)bytes_received >= bytes_needed) {
							all_read = true;
						}
						BMAG("Bytes received: %li\n", (long)bytes_received);
					}
				}

                if (r > 0) { // bytes received

                    PLOG(LSERVER, "Request received from client: <client-address>");
                    MAG("REQUEST: %s\n", client->request);

                    client->request[client->received] = 0; 
                    char* q = strstr(client->request, "\r\n\r\n");

                    // if http response aka (contains \r\n\r\n)
                    /** TODO: switch from Jericho::jscan method to strtok_r */
                    if (q) {
                        if (Jericho::jscan("Connection: keep-alive", client->request)) {
                            client_set_state(client, SOCKST_ALIVE);
                            printf("Setting state\n");
                        } else if (Jericho::jscan("Connection: closed", client->request)) {
                            client_set_state(client, SOCKST_CLOSING);
                            printf("Setting state\n");
                        } else if (Jericho::jscan("Connection: Upgrade", client->request)) {
                            client_set_state(client, SOCKST_UPGRADING);
                            printf("Setting state\n");
                        } else {
                            BRED("INVALID HTTP REQUEST DETECTED\n");
                        }
                    }

                    SocketState state;
					Request request;
					Route route;
					std::string result;
					std::string result2;

                    switch(state = client_get_state(client)) {
                        case SOCKST_ALIVE:
                            // parser2::parse(client, clients);
							if (!is_valid_request(client)) {
								BRED("GET or POST not present!\n");
								char temp[200];
								strncpy(temp, client->request, 199);
								temp[200] = 0;
								BRED("%s\n", temp);
								break;
							}

							if (parse_request(client, &request) < 0) {
								BRED("Invalid request received!");
								break;
							}

							print_request(&request);

							switch (router->protocol(request.path)) {
								case ROUTE_RAW:
									result = router->exec(ROUTE_RAW, request.path, request.args, router, client);
									resource::serve_http(client, clients, result.c_str());	
									break;									
								case ROUTE_SYSTEM:
									BYEL("System...\n");
									result = router->exec(ROUTE_SYSTEM, request.path, request.args, router, client);
									if (result == "TICKET") {
										BBLU("TICKET RECEIVED");
									} else {
										resource::serve_http(client, clients, result.c_str());										
									}
									break;
								case ROUTE_API:
									BYEL("API CALL...\n");
									result = router->exec(ROUTE_API, request.path, request.args, router, client);
									resource::serve_http(client, clients, result.c_str(), std::string("application/json"));
									break;
								case ROUTE_CLUSTER:
									BYEL("DISTRIBUTED CALL...\n");
									result2 = router->execNode(ROUTE_SYSTEM, request.path, {}, router, client);
									memset(client->request, 0, strlen(client->request));
									// resource::serve_http(client, clients, result2.c_str());
									t_write(8080, "./cluster/log/8080.boss", result2.c_str());
									break;
								case ROUTE_NULL:
								case ROUTE_HTTP:
								default:
									BRED("UNREGISTERED ROUTE\n");
									print(router->registry()->securePaths());
								    if (router->secured(request.path)) {
										BRED("ROUTE IS SENSITIVE\n");
										if (authenticate(request.path, request.content)) {
											BRED("ROUTE IS AUTHENTICATED\n");
											result = router->exec(ROUTE_HTTP, request.path, request.args, router, client);
											resource::serve_cxx(client, clients, request.path.c_str());
										} else {
											resource::error(client, "305");
										}
									} else {
										result = router->exec(ROUTE_HTTP, request.path, request.args, router, client);
										resource::serve_cxx(client, clients, request.path.c_str());
									}
									break;
							}

							printf("Route executed. Result is: %s\n", result.c_str());
							printf("Route executed. Result 2 is: %s\n", result2.c_str());
                            // drop_client(client, clients);
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
		// BRED("SERVER LOOP ENDING\n");
    }
}