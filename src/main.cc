#include <signal.h>
#include <sys/ioctl.h>

#include "server/thread_pool.h"
#include "server/server.h"
#include "server/client.h"
#include "picojson.h"
#include "server/router.h"

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

void open(Client* client) {
    char ipBuffer[16];
    client_get_address(client, ipBuffer);
    PLOG(LSERVER, "Opening WS Connection! IP Address: %s\n", ipBuffer);
    BGRE("Opening WS Connection! IP Address: %s\n", ipBuffer);
    ws_enabled = true;
    client_set_state(client, SOCKST_OPEN_WS);
    ws_client = client;
}

void dispatch(Client* conn, ThreadPool* tpool, __ThreadFn fn, Any args) {
    thread_pool_add(tpool, fn, args);
}

void close(Client* client) {
    PLOG(LSERVER, "Closing Connection!");
    char ipBuffer[16];
    client_get_address(client, ipBuffer);
    BRED("Closing Connection! IP Address: %s\n", ipBuffer);
    client_set_state(client, SOCKST_OPEN_STATIC);
    // Client* client = get_client(conn->socket, &clients);
    // drop_client(client, &clients);
}

void message(Client* conn, const unsigned char* message, uint64_t size, int type) {
    PLOG(LSERVER, "Message attempt!");
    char addr_buffer[16];
    client_get_address(conn, addr_buffer);
	printf("I receive a message: %s (size: %ld, type: %d), from: %s\n", message, size, type, addr_buffer);
    
    // will need to find c implementation at some point or make own
    // but using picojson to speed up design considerations process
    picojson::value data;
    std::string s((const char*)message);
    std::string err = picojson::parse(data, s);
    if (!err.empty()) {
        std::cerr << err << std::endl;
    } else {
        printf("json data parsed\n");
    }
    printf("Json data is: %s\n", data.serialize().c_str());
}

int main(int argc, char* argv[]) {
    server_create(argc, argv);

    #ifdef _WIN32
        struct WSAData d;
        if (WSAStartup(MAKEWORD(2, 2), &d)) {
            fprintf(stderr, "Failed to initialize.\n");
        }
    #endif

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        fprintf(stderr, "SSL_CTX_new() failed.\n");
        return 1;
    }

    if (!SSL_CTX_use_certificate_file(ctx, "cert.pem" , SSL_FILETYPE_PEM)
    || !SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM)) {
        fprintf(stderr, "SSL_CTX_use_certificate_file() failed.\n");
        ERR_print_errors_fp(stderr);
        return 1;
    }

    std::vector<std::string> ipAddresses = {};
    SOCKET server = socket_create(0, 8080, 1, AF_INET, SOCK_STREAM); // creates initial socket

    run(&server, &clients, ctx, tpool);

    return 0;
}


void server_create(int argc, char* argv[]) {
    PLOG(LSERVER, "Launching server...");

    // value to thread to avoid deadlock
    sigset_t signals_to_block;
    sigemptyset(&signals_to_block);
    sigaddset(&signals_to_block, SIGINT);
    pthread_sigmask(SIG_BLOCK, &signals_to_block, NULL);

    // create thread to catch shutdown signal
    pthread_create(&shutdown_thread,  
            NULL,
            server_destroy,  
            NULL);

    event_manager.open = &open;
    event_manager.close = &close;
    event_manager.message = &message;
    event_manager.dispatch = &dispatch;

    if (argc < 2) {
        DEBUG("Usage: ./thread_pool <number of threads> <directory>\n");
        exit(1);
    }

    DEBUG("Creating thread pool...\n");
    DEBUG("Number of threads: %i\n", MAX_CONNECTIONS);
    tpool = thread_pool_create(MAX_CONNECTIONS);
    PLOG(LSERVER, "Thread pool created with size: %i\n", MAX_CONNECTIONS);
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
    BYEL("\nShutting down...\n");
    accepting = false;
    thread_info.received_shutdown_req = 1;

    // Wait for in-progress requests threads to finish */
    while (thread_info.num_active > 0) {
        pthread_cond_wait(&thread_info.thread_exit, &thread_info.mutex);
    }

    thread_pool_wait(tpool);
    thread_pool_destroy(tpool);
    pthread_mutex_unlock(&thread_info.mutex);
    
    // BGRE("Gracefully Terminated!\n");
    PLOG(LSERVER, "Gracefully terminated!");
    exit(0);

    return NULL;
}