#ifndef CELERITY_EXTERNAL_API_H_
#define CELERITY_EXTERNAL_API_H_

#include <pthread.h>

#include <set>
#include <unordered_map>
#include <map>
#include <queue>
#include <typeinfo>

#include "util/string_utils.h"
#include "server/defs.h"
#include "server/fetch.h"
#include "server/response.h"
#include "celerity/celerity.h"
#include "cluster/cluster.h"
#include "util/iters.h"
#include "router/route_registry.h"
#include "federate/federator.h"

using namespace Jericho;

typedef void (*WorkerThread)(void*);

/** TODO: Rename to System or Server or Jericho */
class Router {
    RouteRegistry* _registry = nullptr;
    ThreadPool* _tpool;
    WorkerThread _worker;
    Celerity* _celerity;
    Cluster* _cluster;
    Federator* _federator = nullptr;
    bool _needsTrain = false;
    int _fedCounter = 0;

  public:
    Router(ThreadPool* tpool, WorkerThread worker, Cluster* cluster, Celerity* celerity) {
        _registry = new RouteRegistry;
        _worker = worker;
        _tpool = tpool;
        _cluster = cluster;
        _celerity = celerity;
    }

    ~Router() {
        delete _registry;
    }

    const bool needsTrain() const {
        return _needsTrain;
    }

    const bool needsAggregate() const {
        if (_federator == nullptr || _federator == NULL) {
            return false;
        }
        return _federator->active();
    }

    void train(bool train) {
        _needsTrain = train;
    }

    void federate(int numClients, int numRounds, int timeout) {
        _federator = new Federator(numClients, numRounds, timeout, _fedCounter++);
        _federator->start();
    }

    Federator* federator() const {
        return _federator;
    }

    void shutdownFederator() {
        // delete _federator;
        _federator->stop();
        Federator* f = _federator;
        _federator = nullptr;
        BBLU("SHUTTING DOWN FEDERATOR\n");
        delete f;
    }

    void bind(RouteProtocol protocol, std::string path, RouteFunction function) {
        _registry->bind(protocol, path, function);
    }

    void bindNode(std::string path, MessageBuffer* buffer) {
        _registry->bindNode(path, buffer);
    }

    void bindSystem(std::string path, SystemFunction fn, RouteProtocol protocol = ROUTE_SYSTEM) {
        _registry->bindSystem(path, fn, protocol);
    }
    
    RouteProtocol protocol(std::string path) { 
        return _registry->protocol(path);
    }

    void secure(std::string path) {
        _registry->securePath(path);
    }

    const RouteRegistry* registry() const {
        return _registry;
    }

    bool secured(std::string path) {
        return contains(_registry->securePaths(), path);
    }

    WorkerThread worker() const { return _worker; }

    ThreadPool* tpool() const { return _tpool; }

    Celerity* celerity() const { return _celerity; }

    Cluster* cluster() const { return _cluster; }

    std::string exec(RouteProtocol protocol, std::string path, std::unordered_map<std::string, std::string> args, Router* router = NULL, Client* client = NULL) {
        BRED("DOES THIS PRINT\n");
        return _registry->exec(path, args, router, client);
    }

    std::string execNode(RouteProtocol protocol, std::string path, std::unordered_map<std::string, std::string> args, Router* router = NULL, Client* client = NULL) {
        if (client == NULL) {
            return JsonResponse::error(500, "No client to serve!");
        }

        return JsonResponse::error(500, "Exec node deprecated");
    }

    RouteRegistry* registry() { return _registry; }

    void parse(std::string url, Route* route) {
        route->url = url;
        std::string signature = "";
        if (url.find("?") != std::string::npos) {
            std::string::size_type const p(url.find_first_of('?'));
            std::string path = url.substr(0, p);
            std::string args_str = url.substr(p+1, url.size());
            if (path.find("://") != std::string::npos) {
                std::string::size_type const p2(path.find("://"));
                std::string protocol = path.substr(0, p2);
                path = path.substr(p2+3);
                printf("protocol is: %s\n", protocol.c_str());
                route->protocol = protocol;
                printf("path is: %s\n", path.c_str());
                route->path = path;
            }
            std::vector<std::string> dirs = tokenize(path, '/');
            dirs[0] = "/" + dirs[0];
            std::vector<std::string> wildcards = {};
            for (auto d : dirs) {
                if (d.at(0) == ':') {
                    wildcards.push_back(d);
                    signature += "x";
                }
                dirs.push_back(d);
                int sym = _registry->registerSymbol(d);
                signature += std::to_string(sym) + "-";
                printf("dir: %s\n", d.c_str());
            }
            std::vector<std::string> args = tokenize(args_str, '&');
            route->args = args;
            route->wildcards = wildcards;
            for (auto w : wildcards) {
                printf("wc: %s\n", w.c_str());
            }
            for (auto arg : args) {
                printf("arg: %s\n", arg.c_str());
                std::vector<std::string> kv = tokenize(arg, "=");
                route->kvs[kv[0]] = kv[1];
            }
            signature.pop_back();
            route->signature = signature;
            printf("signature: %s\n", signature.c_str());
        }
    }

    void parse_path(std::string path, Route* request) {
        std::string signature = "";
        std::vector<std::string> dirs = tokenize(path, '/');
        // dirs[0] = "/" + dirs[0];
        std::vector<std::string> wildcards = {};
        std::string args_str = "";
        std::string path2 = "";
        if (path.find("?") != std::string::npos) {
            BRED("? found\n");
            std::string::size_type const p(path.find_first_of('?'));
            path2 = path.substr(0, p);
            args_str = path.substr(p+1, path.size());
        } else {
            path2 = path;
        }
        request->path = path2;
        for (auto d : dirs) {
            if (d.size() != 0) {
                if (d.at(0) == ':') {
                    wildcards.push_back(d);
                    signature += "x";
                }
                dirs.push_back(d);
                int sym = _registry->registerSymbol(d);
                signature += std::to_string(sym) + "-";
                printf("dir: %s\n", d.c_str());
            }
        }
        std::vector<std::string> args = {};
        if (args_str != "") {
            args = tokenize(args_str, '&');
        }
        for (auto w : wildcards) {
            printf("wc: %s\n", w.c_str());
        }
        std::set<std::string> vals;
        for (auto arg : args) {
            printf("arg: %s\n", arg.c_str());
            std::vector<std::string> kv = tokenize(arg, "=");
            request->kvs[kv[0]] = kv[1];
            vals.insert(kv[0]);
        }
        if(signature.size() != 0) {
            signature.pop_back();
        }
        for (auto s : vals) {
            printf("-> %s\n", s.c_str());
        }
        request->signature = signature;
        printf("signature: %s\n", signature.c_str());
    }
};

#endif