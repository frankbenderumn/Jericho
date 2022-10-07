#include <signal.h>
#include <sys/ioctl.h>

#include "api/router.h"
#include "api/polygon.h"
#include "server/thread_pool.h"
#include "server/server.h"
#include "server/client.h"
#include "picojson.h"
#include "server/router.h"
#include "server/fetch.h"

typedef struct {
  int             num_active;
  pthread_cond_t  thread_exit;
  pthread_mutex_t mutex;
  int             received_shutdown_req; /* 0=false, 1=true */
} ThreadInfo;

ThreadInfo thread_info;

ThreadPool* tpool;

std::string DIR;

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

    DIR = std::string(argv[3]);
    int port = atoi(argv[1]);

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

    MessageQueue* MQ = new MessageQueue;

    MessageBuffer* buffer = new MessageBuffer;
    buffer->mq = MQ;
    buffer->hostname = "127.0.0.1";
    buffer->port = "8081";
    buffer->sent = "Welcome to the shit show";
    buffer->path = "/";

    MessageBuffer* buffer2 = new MessageBuffer;
    buffer2->mq = MQ;
    buffer2->hostname = "127.0.0.1";
    buffer2->port = "8082";
    buffer2->sent = "Welcome to the best show";
    buffer2->path = "/";

    MessageBuffer* buffer3 = new MessageBuffer;
    buffer3->mq = MQ;
    buffer3->hostname = "127.0.0.1";
    buffer3->port = "8083";
    buffer3->sent = "Welcome to another show";
    buffer3->path = "/";
    

    std::vector<std::string> ipAddresses = {};
    SOCKET server = socket_create(0, port, 1, AF_INET, SOCK_STREAM); // creates initial socket

    Celerity* celerity = new Celerity;
    Router* router = new Router(tpool, fetch, celerity);

    router->bind(ROUTE_API, "/rsi", apiRsi);
    router->bindSystem("/mongo-databases", apiMongoDatabases);
    router->bindSystem("/mongo-insert", apiMongoInsert);
    router->secure("/jericho/denathrius.html");

    if (port == 8080) {
        router->bindNode("/node1", buffer);
        router->bindNode("/node2", buffer2);
        router->bindNode("/node3", buffer3);
    }

    // router->bind("/ohlc", apiOhlc);

    run(&server, &clients, ctx, tpool, router);

    delete buffer;
    delete buffer2;
    delete buffer3;

    delete celerity;
    delete router;
    
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

    if (argc < 3) {
        DEBUG("Usage: ./thread_pool <port> <number of threads> <directory>\n");
        exit(1);
    }

    DEBUG("Creating thread pool...\n");
    DEBUG("Number of threads: %i\n", (size_t)atoi(argv[2]));
    tpool = thread_pool_create((size_t)atoi(argv[2]));
    PLOG(LSERVER, "Thread pool created with size: %i\n", atoi(argv[2]));
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