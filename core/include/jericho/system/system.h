#ifndef CELERITY_EXTERNAL_API_H_
#define CELERITY_EXTERNAL_API_H_

#include <pthread.h>

#include <set>
#include <unordered_map>
#include <map>
#include <queue>
#include <typeinfo>

// #include "util/trace.h"
#include "util/string_utils.h"
#include "server/defs.h"
#include "server/fetch.h"
#include "server/response.h"
#include "server/server.h"
#include "celerity/celerity.h"
#include "cluster/cluster.h"
#include "util/iters.h"
#include "system/router.h"
#include "federator/fl.h"
#include "message/bifrost.h"

using namespace Jericho;

typedef void (*WorkerThread)(void*);

typedef Client* Websocket;

/** TODO: Rename to System or Server or Jericho */
class System {
    Router* _router = nullptr;
    ThreadPool* _tpool;
    WorkerThread _worker;
    Celerity* _celerity;
    Cluster* _cluster;
    FedNode* _federator = nullptr;
    Bifrost* _bifrost = nullptr;
    Websocket _ws = nullptr;
    // Orchestrator* _orch = nullptr;
    bool _needsTrain = false;
    int _fedCounter = 0;
    bool _flash = false;
    bool _fnf = false;
    std::deque<MessageBuffer*> _bufs;

  public:
    size_t num_clients = 0;

    System(ThreadPool* tpool, WorkerThread worker) {
        PCREATE;
        _router = new Router;
        _worker = worker;
        _tpool = tpool;
        // _cluster = cluster;
        // _celerity = celerity;
    }

    System(ThreadPool* tpool, WorkerThread worker, Cluster* cluster, Celerity* celerity) {
        PCREATE;
        _router = new Router;
        _worker = worker;
        _tpool = tpool;
        _cluster = cluster;
        _celerity = celerity;
    }

    ~System() {
        BMAG("Server::~Server: Shutting down server!\n");
        PDESTROY;
        delete _router;
        delete _federator;
        // delete _orch;
        _bifrost->dump();
        delete _bifrost;
    }

    void ws(Websocket sock) { _ws = sock; }
    Websocket ws() const { return _ws; }
    void ws_send(const char* message) {
        if (_ws == nullptr) {
            BRED("Server::ws_send: WS is nullptr!\n");
            return;
        }
        if (strlen(message) >= 2048) {
            // BRED("System::ws_send: Websocket message too long: %li\n", strlen(message));
        }
        BGRE("System::ws_send: SENDING WEBSOCKET...");
        GRE("%s\n", _ws->request);
        // BGRE("System::ws_send: %s\n", message);
        memset(_ws->request, 0, sizeof(_ws->request));
        memcpy(_ws->request, (unsigned char*)message, strlen(message));
        _ws->request[2048] = (unsigned char)'\0';
        thread_pool_add(_tpool, ws_to_client, (void*)_ws); // spawn thread for web socket
    }

    // distributed orm
    void flash(bool val) { _flash = val; }
    const bool flash() const { return _flash; }
    void flashBuffer(std::deque<MessageBuffer*> buf) { _bufs = buf; }
    const std::deque<MessageBuffer*> flashBuffer() const { return _bufs; }

    // federated
    const bool needsTrain() const { return _needsTrain; }
    void train(bool train) { _needsTrain = train; }
    const bool needsAggregate() const {
        // if (_federator == nullptr || _federator == NULL) {
        //     return false;
        // }
        // return _federator->active();
        return false;
    }
    void federate(int numClients, int numRounds, int timeout) {
        // _federator = new Federator(numClients, numRounds, timeout, _fedCounter++);
        // _federator->start();
    }

    void federator(FedNode* fed) { _federator = fed; }
    FedNode* federator() const { return _federator; }

    void celerity(Celerity* celerity) { _celerity = celerity; }
    Celerity* celerity() const { return _celerity; }

    void bifrost(Bifrost* bifrost) { _bifrost = bifrost; }
    Bifrost* bifrost() const { return _bifrost; }

    void shutdownFederator() {
        // delete _federator;
        // _federator->stop();
        // Federator* f = _federator;
        // _federator = nullptr;
        // BBLU("SHUTTING DOWN FEDERATOR\n");
        // delete f;
    }

    WorkerThread worker() const { return _worker; }
    ThreadPool* tpool() const { return _tpool; }
    Cluster* cluster() const { return _cluster; }
    Router* router() const { return _router; }
};

#endif