#include <signal.h>
#include <sys/ioctl.h>

#include "server/thread_pool.h"
#include "server/server.h"
#include "server/client.h"
#include "picojson.h"

typedef struct {
  int             num_active;
  pthread_cond_t  thread_exit;
  pthread_mutex_t mutex;
  int             received_shutdown_req; /* 0=false, 1=true */
} ThreadInfo;

ThreadInfo thread_info;

ThreadPool* tpool;
int job_ct = 0;

int received_shutdown_req = 0;
pthread_mutex_t shutdown_lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_t shutdown_thread;
static bool ws_enabled = false;
static Client* ws_client;

int retVal = 0;
int* exitVal = 0;

volatile bool accepting = true;

void server_create(int argc, char* argv[]);
Any server_destroy(void *arg);

static Client* clients = 0;
// static Peer* peer = 0;

void open(Client* client) {
    PLOGV(LSERVER, "WEBSOCKET", 5, "Opening Connection!");
    char ipBuffer[16];
    client_get_address(client, ipBuffer);
    DEBUG("Opening Connection! IP Address: %s\n", ipBuffer);
    client_set_state(client, SOCKST_OPEN_WS);
}

void dispatch(Client* conn, ThreadPool* tpool, __ThreadFn fn, Any args) {
    thread_pool_add(tpool, fn, args);
}

void close(Client* client) {
    PLOGV(LSERVER, "WEBSOCKET", 5, "Closing Connection!");
    char ipBuffer[16];
    client_get_address(client, ipBuffer);
    DEBUG("Closing Connection! IP Address: %s\n", ipBuffer);
    client_set_state(client, SOCKST_OPEN_STATIC);
    // Client* client = get_client(conn->socket, &clients);
    // drop_client(client, &clients);
}

void command(const char* str) {
    if (0 == strcmp(str, "expand")) {

    }
}

void message(Client* conn, const unsigned char* message, uint64_t size, int type) {
    char addr_buffer[16];
    client_get_address(conn, addr_buffer);
	// printf("message: %s (size: %ld, type: %d), from: %s\n", message, size, type, addr_buffer);
    PLOGV(LSERVER, "WEBSOCKET", 2, "message: %s (size: %ld, type: %d), from: %s", message, size, type, addr_buffer);
// will need to find c implementation at some point or make own
// but using picojson to speed up design considerations process
    picojson::value data;
    std::string s((const char*)message);
    std::string picoerr = picojson::parse(data, s);
    if (!picoerr.empty()) {
        std::cerr << picoerr << std::endl;
    } else {
        // printf("json data parsed\n");
        if (data.is<picojson::object>()) {
            picojson::object o = data.get<picojson::object>();
            int j = 0;
            for (picojson::object::const_iterator it = o.begin(); it != o.end(); it++) {
                // std::cout << j << " " << it->first << ": " << it->second << std::endl;
                j++;
                if (it->first == "command") {
                    command(it->second.serialize().c_str());
                }
            }
        } else if (data.is<picojson::array>()) {    
            picojson::array arr = data.get<picojson::array>();
            for (int i = 0; i < arr.size(); i++) {
                picojson::value v = arr[i];
                if (v.is<picojson::object>()) {
                    picojson::object o2 = v.get<picojson::object>();
                    int j2 = 0;
                    for (picojson::object::const_iterator it2 = o2.begin(); it2 != o2.end(); it2++) {
                        // std::cout << j2 << " " << it2->first << ": " << it2->second << std::endl;
                        j2++;
                    }
                } else if (v.is<double>()) {
                    double d2 = v.get<double>();
                // } else if (v.is<int>()) {
                //     int i2 = v.get<int>();
                } else if (v.is<std::string>()) {
                    std::string s2 = v.get<std::string>(); 
                } else if (v.is<bool>()) {
                    bool b2 = v.get<bool>();
                }
            }
        } else {
            PLOGV(LSERVER, "WS_REQUEST", 1, "Unknown websocket request: %s\n", data.serialize().c_str());
        }
    }
    // printf("Json data is: %s\n", data.serialize().c_str());
}

/** TODO: switch to large scale response parse (send_response(code, socket)) */ 
void parse_response(Client* client) {
    DEBUG("request in parse: %s\n", client->request);
    int g = strncmp("GET /", client->request, 5);
    int p = strncmp("POST /", client->request, 6);
    if (g && p) {
        send_400(client);
    } else {
        if (!g) {
            char* path = client->request + 4; // removes "GET "
            char* end_path = strstr(path, " "); // finds first occurence of " "
            if (!end_path) {
                send_400(client); // none terminating path
            } else {
                *end_path = 0; // zero out char
                serve_resource(client, path); // static file serving
            }
        } else if (!p) {
            post_resource(client, client->request);
        }
    }
}

int main(int argc, char* argv[]) {
    #ifdef _WIN32
        struct WSAData d;
        if (WSAStartup(MAKEWORD(2, 2), &d)) {
            // fprintf(stderr, "Failed to initialize.\n");
        }
    #endif

    if (argc != 4) {
        RED("Usage: ./bin/c.sh <PORT> <FLAGS>\n");
        exit(1);
    }
    server_create(argc, argv);

    const char* port = argv[1];
    int iport = atoi(port);

    Peer* main = (Peer*)malloc(sizeof(Peer));
    printf("Creating main peer!\n");
    peer_create(main, "127.0.0.1", port, 10230);
    // peer(&main, "127.0.0.1", port, 10230, "JMPP:\r\n\r\nGreetings traveller!");
    BLU("main peer hostname: %s\n", main->hostname);
    Cluster* cluster = (Cluster*)malloc(sizeof(Cluster));
    printf("Creating cluster!\n");
    cluster_create(cluster, main, getenv("CLUSTER"));
    printf("Cluster created!\n");
    YEL("Cluster precheck...\n");
    Peer* head = cluster->slaves;
    int j = 0;
    while (head != NULL && j != cluster->numSlaves) {
        printf("Slave %i: hostname: %s, port: %s, signature: %i\n", j, head->hostname, head->port, head->signature);
        head = head->next;
        j++;
    }

    bool synchronized = true;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        // fprintf(stderr, "SSL_CTX_new() failed.\n");
        return 1;
    }


    if (!SSL_CTX_use_certificate_file(ctx, "cert.pem" , SSL_FILETYPE_PEM)
    || !SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM)) {
        // fprintf(stderr, "SSL_CTX_use_certificate_file() failed.\n");
        // ERR_print_errors_fp(stderr);
        return 1;
    }

    SOCKET server = socket_create(0, iport, 1, AF_INET, SOCK_STREAM); // creates initial socket
    printf("Listening on ");
    BLU("https://%s:%i\n", "127.0.0.1", iport);

    while (1) {

        if (!synchronized) {
            int j = 0;
            int k = 0;
            Peer* head = cluster->slaves;
            while (head != NULL && j != cluster->numSlaves) {
                PeerState state = peer_get_state(head);
                // switch(state) {
                //     case PEERST_NULL:
                //         BYEL("Peer state is null!\n");
                thread_pool_add(tpool, peer, (void*)head);

                if (state == PEERST_ONLINE) {
                    BGRE("SYNCHRONIZED!\n")
                    synchronized = true;
                }
            //             break;
            //         case PEERST_FAILED:
            //             BRED("Peer state failed!\n");
            //             printf("%s\n", head->sendQueue);
            //             thread_pool_add(tpool, peer, (void*)head);
            //             break;
            //         default:
            //             k++;
            //             BGRE("Peer already synced!\n");
            //             break;
            //     }
            //     if (k == cluster->numSlaves) {
            //         BGRE("Cluster synchronized!\n\n\n\n\n");
            //         synchronized = true;
            //     }
                head = head->next;
            }
        }

        fd_set reads;
        reads = wait_on_clients(server, &clients);

        if (FD_ISSET(server, &reads)) {
            Client* client = get_client(-1, &clients);

            client->socket = accept(server, (struct sockaddr*) &(client->address), &(client->address_length));

            if (!ISVALIDSOCKET(client->socket)) {
                PFAIL(ECONN, "accept() failed. (%d)\n", SOCKERR());
            }

            char address_buffer[16];
            client_get_address(client, address_buffer);
            DEBUG("New connection from %s.\n", address_buffer);

            struct sockaddr_in client_in;
            socklen_t clientsz = sizeof(client);
            getsockname(client->socket, (struct sockaddr *) &client_in, &clientsz);
            printf("[%s:%u] > ", inet_ntoa(client_in.sin_addr), ntohs(client_in.sin_port));
            client->port = ntohs(client_in.sin_port);

            client->ssl = SSL_new(ctx);
            if (!client->ssl) {
                // fprintf(stderr, "SSL_new() failed.\n");
                return 1;
            }

            SSL_set_fd(client->ssl, client->socket);
            if (SSL_accept(client->ssl) != 1) {
                //SSL_get_error(client->ssl, SSL_accept(...));

                // will print even if not failing due to time taken to read SSL
                // ERR_print_errors_fp(stderr);
                drop_client(client, &clients);
            } else {
                DEBUG("New connection from %s.\n",
                        "localhost");

                PLOGV(LSERVER, "CONNECTION", 5, "New connection from %s", "127.0.0.1");

                DEBUG("NUM CLIENTS: %i\n", (int)(sizeof(clients)/sizeof(Client)));

                DEBUG("SSL connection using %s\n",
                        SSL_get_cipher(client->ssl));
            }
        }

        Client* client = clients;

        while (client) {
            // iterate through clients
            Client* next = client->next;

            if (FD_ISSET(client->socket, &reads)) {
                // max request
                if (MAX_REQUEST_SIZE == client->received) {
                    DEBUG("MAX REQUEST SIZE\n");
                    char address_buffer[16];
                    client_get_address(client, address_buffer);
                    PLOGV(LSERVER, "ERROR", 1, "Max request size reached from client %s\n", address_buffer);
                    send_400(client);
                    client = next;
                    continue;
                }

                /** STUB: do send before receive for fault tolerance? */

                // receives bytes from client and asserts against request limit
                int r = SSL_read(client->ssl, client->request + client->received, MAX_REQUEST_SIZE - client->received); 

                if (r > 0) { // bytes received

                    // PLOGV(LSERVER, "REQUEST", 5, "%s\n", client->request);
                    DEBUG("Request: %s\n", client->request);

                    client->received += r; // increment bytes received
                    client->request[client->received] = 0; 
                    char* q = strstr(client->request, "\r\n\r\n");

                    // printf("q is: %s\n", q);

                    // if http response aka (contains \r\n\r\n)
                    /** TODO: switch from scan method to strtok_r */
                    if (q) {
                        if (scan("Connection: keep-alive", client->request)) {
                            client_set_state(client, SOCKST_ALIVE);
                            DEBUG("Setting state\n");
                        } else if (scan("Connection: closed", client->request)) {
                            client_set_state(client, SOCKST_CLOSING);
                            DEBUG("Setting state\n");
                        } else if (scan("Connection: Upgrade", client->request)) {
                            client_set_state(client, SOCKST_UPGRADING);
                            DEBUG("Setting state\n");
                        } else if (scan("JMPP/1.0", client->request)) {
                            client_set_state(client, SOCKST_PEER);
                            PLOGV(LSERVER, "PEER", 5, "Incoming peer message!");
                        } else {
                            DEBUG("INVALID HTTP REQUEST DETECTED\n");
                        }
                    }

                    SocketState state;
                    const char* peer_response;
                    switch(state = client_get_state(client)) {
                        case SOCKST_ALIVE:
                            parse_response(client);
                            memset(client->request, 0, strlen(client->request));
                            break;
                        case SOCKST_CLOSING:
                            DEBUG("DROPPING CLIENT!\n");
                            drop_client(client, &clients);
                            break;
                        case SOCKST_UPGRADING: 
                            DEBUG("ATTEMPTING TO UPGRADE TO WEBSOCKET!\n");
                            DEBUG("Socket fd: %i\n", client->socket);
                            DEBUG("Client request: %s\n", client->request);
                            // connect((void*)client); non threaded version
                            thread_pool_add(tpool, connect, (void*)client);
                            break;
                        case SOCKST_OPEN_WS:
                            DEBUG("Client request: %s\n", (unsigned char*)client->request);
                            thread_pool_add(tpool, recv_websocket, (void*)client);
                            break;
                        case SOCKST_PEER:
                            printf("Peer request from: %i\n", client->port);
                            printf("Requested: %s\n", client->request);
                            peer_response = "JMPP/1.0\r\nConnection: established\r\n\r\nLink was acknowldeged";
                            SSL_write(client->ssl, peer_response, strlen(peer_response));
                            break;
                        default:
                            DEBUG("AN UNEXPECTED STATE WAS ENCOUNTERED!\n");
                            break;
                    }
                }
            }
            client = next;
        }
    }
    return 0;
}


void server_create(int argc, char* argv[]) {
    PINIT
    const char* host = "127.0.0.1";
    const char* port = "8082";
    int reuse = 1;
    int retVal = 0;
    YEL("Conducting server prechecks...\n");

    // intialize http headers log
    for (int i = 0; i < H_END; i++) {
        // do nothing for now,
        // use later for debug enhancement 
        // and testing
    }

    sigset_t signals_to_block;
    // value to thread to avoid deadlock
    sigemptyset(&signals_to_block);
    sigaddset(&signals_to_block, SIGINT);
    pthread_sigmask(SIG_BLOCK, &signals_to_block, NULL);

    /* create thread to catch shutdown signal */  
    pthread_create(&shutdown_thread,  
            NULL,
            server_destroy,  
            NULL);

    event_manager.open = &open;
    event_manager.close = &close;
    event_manager.message = &message;
    event_manager.dispatch = &dispatch;

    if (argc < 2) {
        red(); printf("Usage: ./thread_pool <number of threads> <directory>\n"); clearcolor();
        exit(1);
    }

    DEBUG("Creating thread pool...\n");
    DEBUG("Number of threads: %i\n", MAX_CONNECTIONS);
    tpool = thread_pool_create(MAX_CONNECTIONS);
    DEBUG("Thread pool created!\n");
}

/** WARN: not platform independent */
Any server_destroy(void *arg) {
    sigset_t signals_to_catch;
    int caught;
    
    // Wait for SIGUSR1 
    sigemptyset(&signals_to_catch);
    sigaddset(&signals_to_catch, SIGINT);

    sigwait(&signals_to_catch, &caught);

    // got SIGUSR1 -- start shutdown
    pthread_mutex_lock(&thread_info.mutex);
    DEBUG("\nShutting down...\n");
    accepting = false;
    thread_info.received_shutdown_req = 1;

    // Wait for in-progress requests threads to finish */
    while (thread_info.num_active > 0) {
        pthread_cond_wait(&thread_info.thread_exit, &thread_info.mutex);
    }

    thread_pool_wait(tpool);
    thread_pool_destroy(tpool);
    pthread_mutex_unlock(&thread_info.mutex);
    BGRE("Gracefully Terminated!\n");
    PLOGV(LSERVER, "SHUTDOWN", 2, "Gracefully terminated!");
    PSHUTDOWN
    exit(0);

    return NULL;
}