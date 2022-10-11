#include "server/client.h"

// re-entrant safe
void client_get_address(struct Client* ci, char* dest) {
    char address_buffer[16]; // char array to store IP address, static so erased after function termination
    getnameinfo((struct sockaddr*)&ci->address, ci->address_length, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
    strncpy(dest, address_buffer, INET_ADDRSTRLEN);
}

// re-entrant safe ? (needs to be static for c)
SocketState client_get_state(Client* client) {
    SocketState state;
    pthread_mutex_lock(&(client->state_mutex));
    state = client->state;
    pthread_mutex_unlock(&(client->state_mutex));
    return state;
}

void client_set_grade(Client* client, bool grade) {
    pthread_mutex_lock(&(client->grade_mutex));
    client->grade = grade;
    pthread_mutex_unlock(&(client->grade_mutex));
}

bool client_get_grade(Client* client) {
    bool grade;
    pthread_mutex_lock(&(client->grade_mutex));
    grade = client->grade;
    pthread_mutex_unlock(&(client->grade_mutex));
    return grade;
}

void client_set_state(Client* client, SocketState state) {
    pthread_mutex_lock(&(client->state_mutex));
    client->state = state;
    pthread_mutex_unlock(&(client->state_mutex));
}

static int CLIENT_ID = -1;

// finds exisiting client or creates new one if does not exist
Client* get_client(SOCKET s, Client** clients) {
    Client* ci = *clients; // pointer to linked list

    while (ci) {
        if (ci->socket == s) break; // found client
        ci = ci->next; // iterate
    }

    if (ci) return ci; // last client

    // allocate 1 new client with block size of Client (calloc actually initializes to 0 unlike malloc)
    Client* n = (Client*) calloc(1, sizeof(Client)); 
    n->id = ++CLIENT_ID;

    if (!n) { PFAIL(ENEM, "Out of memory!"); } // not enough memory

    // set address length to size of sockaddr_storage
    n->address_length = sizeof(n->address);
    n->next = *clients; // append to front of linked list
    // n->state = SOCKST_NULL;
    // n->grade = false;
    // n->last_pong_id = -1;
    // n->current_ping_id = -1;
    // n->close_thread = false;
    // if (pthread_mutex_init(&n->state_mutex, NULL)) {
    //     PERR(ESERVER, "Failed to initialize state mutex on client!"); }
    // if (pthread_mutex_init(&n->grade_mutex, NULL)) {
    //     PERR(ESERVER, "Failed to initialize upgrade mutex on client!"); }
    // if (pthread_mutex_init(&n->send_mutex, NULL)) {
    //     PERR(ESERVER, "Failed to initialize send mutex on client!"); }
    // if (pthread_mutex_init(&n->ping_mutex, NULL)) {
    //     PERR(ESERVER, "Failed to initialize ping mutex on client!"); }
    // if (pthread_cond_init(&n->state_close_cond, NULL)) {
    //     PERR(ESERVER, "Failed to initialize conditional mutex on client!"); }

    *clients = n; // set index to front of clients
    return n; 
}

void drop_client(Client* client, Client** clients) {
    if (!client->promised) {
        SSL_shutdown(client->ssl);
        CLOSESOCKET(client->socket); // kill connection
        SSL_free(client->ssl);

        Client*** p = &clients; // pointer-to-pointer
        
        // double pointer helps with case of dropped client at head of list
        while (**p) { 
            if (**p == client) { // if client
                **p = client->next; // set pointer to client
                free(client); // free memory, was allocated on heap
                PLOG(LSERVER, "Dropping client: <client-address>");
                BYEL("Client dropped!\n");
                return;
            }
            *p = &(**p)->next; // iterate
        }

        PFAIL(ESERVER, "Error dropping client -- client not found!");
    } else {
        BYEL("Client promised can not drop!\n");
    }
}

// blocking wait until new client or all packets from client received
fd_set wait_on_clients(SOCKET server, Client** clients) {
    fd_set reads; // set of file descriptors
    FD_ZERO(&reads); // set zero bits for all descriptors
    FD_SET(server, &reads); // server fd bit
    SOCKET max_socket = server; // current num of sockets
    Client* ci = *clients; // list of clients

    while (ci) {
        FD_SET(ci->socket, &reads); // set fd bit of socket
        if (ci->socket > max_socket) { // determine if max socket
            max_socket = ci->socket; // set new max
        }
        ci = ci->next; // iterate
    }

    // waits for sockets in fd_set to be available for access
    // params: (num of fds + 1, read fds, write fds, exceptional conditions fds, timeout for blocking)
    // use pselect with 6th param for sigmask to ignore certain signals on threads
    if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
        // fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
        // exit(1);
        PFAIL(ESERVER, "select() failed");
    }

    return reads;
}

void close_client(Client* client, int lock) {
	if (!ISVALIDSOCKET(client)) return;

	client_set_state(client, SOCKST_CLOSED);

	socket_close(client->socket);

	/* Destroy client mutexes and clear fd 'slot'. */
	/* clang-format off */
	if (lock)
		pthread_mutex_lock(&global_mutex);
			client->socket = -1;
			pthread_cond_destroy(&client->state_close_cond);
			pthread_mutex_destroy(&client->state_mutex);
			pthread_mutex_destroy(&client->send_mutex);
			pthread_mutex_destroy(&client->ping_mutex);
	if (lock)
		pthread_mutex_unlock(&global_mutex);
	/* clang-format on */
}

int ws_close_client(Client *client) {
	unsigned char clse_code[2];
	int cc;

	/* Check if client is a valid and clientected client. */
	if (!ISVALIDSOCKET(client) || client->socket == -1)
		return (-1);

	/*
	 * Instead of using do_close(), we use this to avoid using
	 * payload buffer from wfd and avoid a race condition
	 * if this is invoked asynchronously.
	 */
	cc = WS_CLSE_NORMAL;
	clse_code[0] = (cc >> 8);
	clse_code[1] = (cc & 0xFF);
	if (ws_sendframe(client, (const char *)clse_code, sizeof(char) * 2, FRAME_CLOSE) < 0) {
		PERR(ESERVER, "An error has occurred while sending closing frame!\n");
		return -1;
	}

	/*
	 * Starts the timeout thread: if the client did not send
	 * a close frame in TIMEOUT_MS milliseconds, the server
	 * will close the connection with error code (1002).
	 */
	start_close_timeout(client);
	return 0;
}
