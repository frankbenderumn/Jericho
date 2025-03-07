#include <signal.h>
#include <sys/ioctl.h>

#include "system/system.h"
#include "api/apis.h"
#include "server/thread_pool.h"
#include "server/server.h"
#include "server/client.h"
#include "picojson.h"
#include "server/fetch.h"
#include "celerity/celerity.h"
#include "celerity/entity/internal/query_ir.h"
#include "system/routes.h"
#include "module/migrator.h"
#include "module/module.h"
#include "message/bifrost.h"
#include "util/trace.h"

typedef struct {
  int             num_active;
  pthread_cond_t  thread_exit;
  pthread_mutex_t mutex;
  int             received_shutdown_req; /* 0=false, 1=true */
} ThreadInfo;

ThreadInfo thread_info;

ThreadPool* tpool;

std::string PUBLIC_DIRECTORY;

int job_ct = 0;

int received_shutdown_req = 0;
pthread_mutex_t shutdown_lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_t shutdown_thread;
static bool ws_enabled = false;
static Client* ws_client;

int retVal = 0;
int* exitVal = 0;

volatile bool accepting = true;

// some potentially unecessary global variables
System* sys = nullptr;
Orchestrator* orch = nullptr;

void server_create(int argc, char* argv[]);
Any server_destroy(void *arg);
SSL_CTX* ssl_init();

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

void atexit_fish() {
    TRACESCAN;
}

int main(int argc, char* argv[]) {
    const int result_1 = std::atexit(atexit_fish);

    if (std::string(argv[3]).find("..") != std::string::npos) {
        BRED("Invalid directory name!");
        return 1;
    }

    std::string dbname(argv[4]);
    std::string flag(argv[5]);
    std::string script(argv[6]);
    std::string portStr(argv[1]);
    std::string host = "127.0.0.1";
    int port = atoi(argv[1]);
    PUBLIC_DIRECTORY = "./public/"+std::string(argv[3]);

    server_create(argc, argv);

    SSL_CTX* ctx = ssl_init();
    if (ctx == NULL) { 
        BRED("FAILED TO INITALIZE SSL CTX\n");
        return 1; 
    }

    Bifrost* bifrost = new Bifrost(host, portStr, tpool, fetch);
    ClusterIndex* index = new ClusterIndex;
    ClusterNode* boss = new ClusterNode("127.0.0.1", std::string(argv[1]), "./public/"+std::string(argv[3]), index);
    Cluster* cluster = new Cluster(CLUSTER_MAIN, boss, index);

    SOCKET server = socket_create(0, port, 1, AF_INET, SOCK_STREAM);

    sys = new System(tpool, fetch, cluster, NULL);
    sys->bifrost(bifrost);

    FedNode* federator;
    Celerity* celerity;
    celerity = Module::celerity();
    federator = Module::federator(script, host + ":" + argv[1]);
    if (celerity) sys->celerity(celerity);
    if (federator != nullptr) {
        BGRE("Federator bound!\n");
        sys->federator(federator);
    } else {
        BRED("Federator is nullptr!\n");
    }
    
    compile_routes(sys->router());
    BBLU("==========================================\n");
    BBLU("               SERVER START               \n");
    BBLU("==========================================\n");

    if (flag != "synch" && flag != "migrate") {
        run(&server, &clients, ctx, tpool, sys);
    }
    
    return 0;
}

SSL_CTX* ssl_init() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    SSL_CTX* ctx = NULL;

    // ask chatgpt for difference
    ctx = SSL_CTX_new(TLS_server_method());
    // ctx = SSL_CTX_new(TLS_method());
    if (!ctx) {
        fprintf(stderr, "SSL_CTX_new() failed.\n");
        return NULL;
    }

    if (!SSL_CTX_use_certificate_file(ctx, "cert.pem" , SSL_FILETYPE_PEM)
    || !SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM)) {
        fprintf(stderr, "SSL_CTX_use_certificate_file() failed.\n");
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    return ctx;
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

#ifdef _WIN32
    struct WSAData d;
    if (WSAStartup(MAKEWORD(2, 2), &d)) {
        fprintf(stderr, "Failed to initialize.\n");
    }
#endif

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

    Client* cli = clients;
    size_t num_clis = 0;
	while(cli != NULL) {
		BMAG("Need to delete client here!\n");
        drop_client(cli, &clients, &num_clis);
		cli = cli->next;
	}

    BYEL("Deleting system...\n");
    delete orch;
    delete sys;
    
    // BGRE("Gracefully Terminated!\n");
    PLOG(LSERVER, "Gracefully terminated!");
    exit(0);

    return NULL;
}