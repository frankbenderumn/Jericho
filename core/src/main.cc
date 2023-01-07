#include <signal.h>
#include <sys/ioctl.h>

#include "router/router.h"
#include "api/apis.h"
#include "server/thread_pool.h"
#include "server/server.h"
#include "server/client.h"
#include "picojson.h"
#include "server/fetch.h"
#include "celerity/celerity.h"
#include "celerity/entity/internal/query_ir.h"
#include "router/routes.h"
#include "migrator/migrator.h"
#include "message/bifrost.h"

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

int main(int argc, char* argv[]) {
    if (std::string(argv[3]).find("..") != std::string::npos) {
        BRED("Invalid directory name!");
        return 1;
    }

    std::string dbname(argv[4]);
    std::string flag(argv[5]);
    std::string script(argv[6]);
    std::string portStr(argv[1]);
    std::string host = "127.0.0.1";

    bool topology = false;
    bool dbCluster = false;
    bool federator = false;

    Orchestrator* orch = nullptr;
    BWHI("SIZE OF ORCHESTRATOR: %li\n", sizeof(orch));
    Celerity* celerity = nullptr;
    Bifrost* bifrost = new Bifrost;

    if (flag != "synch") {
        // celerity = new Celerity(dbname);

        // std::string schema = celerity->serialize();
        // BMAG("SCHEMA: %s\n", schema.c_str());

        // User user;
        // Users users = user.find_by("username", "tankinfranklin").commit(celerity->primary()).as<Users>(); 
        // BBLU("Users size: %i\n", (int)users.size());

        // Artist artist(name_("Jerry Jones"));
        // artist->persist(celerity->primary());
    }

    if (flag == "migrate") {
        celerity = new Celerity(dbname);
        std::string celerity_path = "./ext/celerity/core/include/celerity/entity/";
        // std::string model = "./db/";
        celerity->migrate(script.c_str(), celerity_path.c_str());
    }

    server_create(argc, argv);

    PUBLIC_DIRECTORY = "./public/"+std::string(argv[3]);
    int port = atoi(argv[1]);

    SSL_CTX* ctx = ssl_init();
    if (ctx == NULL) { 
        BRED("FAILED TO INITALIZE SSL CTX\n");
        return 1; 
    }

    ClusterIndex* index = new ClusterIndex;
    ClusterNode* boss = new ClusterNode("127.0.0.1", std::string(argv[1]), "./public/"+std::string(argv[3]), index);
    Cluster* cluster = new Cluster(CLUSTER_MAIN, boss, index);

    SOCKET server = socket_create(0, port, 1, AF_INET, SOCK_STREAM);

    Router* router = new Router(tpool, fetch, cluster, celerity);
    router->bifrost(bifrost);
    
    if (script != "orch") {
        // orch = new Orchestrator;
        // if (migrate(host+":"+portStr, orch, script) < 0) {
        //     BRED("Main.cc: Failed to migrate!: %s\n", script.c_str());
        //     return 1;
        // }
        // // SEGH
        // printf("What the fuck\n");
        // if (orch->federator == nullptr) {
        //     BMAG("Orch federator is null somehow\n");
        //     exit(1);
        // }
        // printf("What the fuck\n");
        // orch->federator->dump();
        // printf("I want to cry\n");
        // FedRole role = orch->federator->local()->role();
        // printf("It does not make sense\n");
        // if (role == FED_ROLE_CENTRAL || role == FED_ROLE_AGGREGATOR) {
        //     router->cluster()->boss()->configDir("agg");
        // }
        // router->federator(orch->federator);
        // write_file("./log/debug.log", "Hello friend");
    }

    if (flag == "synch") {
        BYEL("SYNCHING...\n");
        router->flash(true);
        std::deque<MessageBuffer*> bufs;
        for (auto db : orch->dbs) {
            BMAG("name: %s, url: %s\n", db.second.c_str(), db.first.c_str());
            std::vector<std::string> toks = prizm::tokenize(db.first, ":");
            MessageBuffer* buf = new MessageBuffer;
            buf->hostname = toks[0];
            buf->port = toks[1];
            buf->fromPort = std::string(argv[1]);
            buf->dir = PUBLIC_DIRECTORY;
            buf->path = "/db-info";
            buf->sent = "Requesting db info!";
            bufs.push_back(buf);
        }
        router->flashBuffer(bufs);
    }

    compile_routes(router);

    BBLU("RUNNING...\n");

    if (flag != "synch" && flag != "migrate") {
        run(&server, &clients, ctx, tpool, router);
    } else {
		if (router->flash()) {
			BYEL("FLASHING...\n");
			router->cluster()->boss()->brokerBroadcast(router, NULL, router->flashBuffer(), group_callback);
		}

        while (1) {
        	MessageBroker* broker = router->cluster()->boss()->poll(NULL);
			if (broker != NULL) {
				BGRE("FIRING ASYNC CALL\n");
				std::string response = broker->callback()(router, NULL, broker->response(NULL), broker->callbackType(), NULL);
				if (broker->epoch() == 0) {
					// if (isHTTP(response)) {
					// 	resource::serve_raw(client, clients, response.c_str());
					// } else {
					// 	resource::serve_http(client, clients, response.c_str());
					// }
					// client->promised = false;
					// BBLU("I DONT UNDERSTAND: %s\n", response.c_str());
					// drop_client(client, clients); // this segfaults buffer size of 500000 but not 4095 (fixed)
					break;
				}
			}
        }
    }

    BYEL("SHUTTING DOWN ---\n");

    if (celerity != nullptr) {
        delete celerity;
    }

    delete cluster;
    delete router;

    if (orch != nullptr) {
        delete orch;
    }
    
    return 0;
}

SSL_CTX* ssl_init() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    SSL_CTX* ctx = NULL;

    ctx = SSL_CTX_new(TLS_server_method());
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
    
    // BGRE("Gracefully Terminated!\n");
    PLOG(LSERVER, "Gracefully terminated!");
    exit(0);

    return NULL;
}