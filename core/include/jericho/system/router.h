#ifndef JERICHO_ROUTER_ROUTE_REGISTRY_H_
#define JERICHO_ROUTER_ROUTE_REGISTRY_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "server/response.h"
#include "server/defs.h"
#include "util/trace.h"
#include "session/session.h"

#include "system/route_functions.h"

class MessageBuffer;
class System;

enum RouteProtocol {
    ROUTE_NULL,
    ROUTE_HTTP,
    ROUTE_API,
    ROUTE_CLUSTER,
    ROUTE_SYSTEM,
    ROUTE_JERICHO,
    ROUTE_SECURE,
    ROUTE_CONTROLLER,
    ROUTE_RESOURCE,
    ROUTE_DB,
    ROUTE_RAW,
    ROUTE_JSON,
    ROUTE_XML,
    ROUTE_YAML,
    ROUTE_CSV,
    ROUTE_IRIS
};

struct Route {
    std::string url;
    std::string path;
    std::string protocol;
    std::vector<std::string> wildcards;
    std::vector<std::string> args;
    std::unordered_map<std::string, std::string> kvs;
    std::string signature;
    RouteFunction callback = 0;
    std::vector<ContractFunction> pre = {};
    std::vector<ContractFunction> post = {};
    ResourceFunction resource = 0;
    bool secure = false;
    bool iris = false;
    RouteProtocol type = ROUTE_NULL;
    Route(RouteProtocol protocol, std::string path, RouteFunction fn) { 
        PCREATE;
        this->type = protocol;
        this->path = path;
        this->callback = fn;
    }
    ~Route() { PDESTROY; }
    std::string exec(Request* req, System* sys = NULL, Client* cli = NULL, MessageBroker* mb = NULL) {
        // for (auto p : pre) {
        //     if (p(args) < 0) {
        //         return "undefined";
        //     }
        // }
        std::string result = "No callback";
        if (callback != NULL) {
            BYEL("Callback exists!\n");
            result = callback(req, sys, cli, mb);
        }
        // for (auto p : post) {
        //     if (p(args) < 0) {
        //         return "undefined";
        //     }
        // }
        return result;
    }
    void dump() {
BMAG("=====================================================================\n");
MAG("Route Dump\n");
// MAG("Url: %s\n", url.c_str());
MAG("Path: %s\n", path.c_str());
std::string typeStr;
switch(this->type) {
    case ROUTE_HTTP:
        typeStr = "HTTP";
        break;
    case ROUTE_SYSTEM:
        typeStr = "System";
        break;
    case ROUTE_API:
        typeStr = "Api";
        break;
    case ROUTE_IRIS:
        typeStr = "Iris";
        break;
    case ROUTE_JERICHO:
        typeStr = "Jericho";
        break;
    case ROUTE_DB:
        typeStr = "Database";
        break;
    case ROUTE_CONTROLLER:
        typeStr = "Controller";
        break;
    default:
        typeStr = "NULL";
        break;
}
MAG("Type: %s\n", typeStr.c_str());
// WHI("Implement wildcards? Could be a vulnerability but need interpolation\n");
// WHI("Implement route callbacks? \n");
    }
};

static int wc_symbol_id = -1;

static int MESSAGE_ID = -1;

class Router {
    std::unordered_map<std::string, int> _symbols;
    std::unordered_map<std::string, RouteProtocol> _routeProtocols;
    std::unordered_map<std::string, Route*> _routeTable;
    std::unordered_map<std::string, Route*> _secureTable;
    std::unordered_map<std::string, RouteFunction> _routeFunctions;
    std::unordered_map<std::string, RouteFunction> _httpFunctions;
    // std::unordered_map<std::string, RouteFunction> _systemFunctions;
    std::unordered_map<std::string, MessageBuffer*> _nodes;
    std::unordered_map<std::string, std::string> _ipaths;
    std::unordered_map<std::string, std::string> _test;
    std::vector<std::string> _securePaths;
    std::vector<Route*> _routes;

  public:
    Router() { PCREATE; }

    ~Router() {
        PDESTROY;
        for (auto route : _routes) {
            delete route;
        }
    }

    /** TODO: separate to Route class? */
    bool hasFunction(std::string path);

    bool hasRoute(std::string route);

    bool hasSymbol(std::string symbol);

    bool hasNode(std::string path);

    void registerRoute(std::string routeName, Route* route);

    void securePath(std::string path, RouteFunction fn = NULL);

    const std::vector<std::string>& securePaths() const;

    RouteProtocol protocol(std::string path);

    void bind(RouteProtocol protocol, std::string path, RouteFunction function, std::vector<ContractFunction> pre = {}, std::vector<ContractFunction> post = {});

    void ipath(std::string path, std::string ipath);

    std::string ipath(std::string path);

    bool contains(std::string path);

    void bindSystem(std::string path, SystemFunction function, RouteProtocol protocol = ROUTE_SYSTEM);

    void bindNode(std::string path, MessageBuffer* buffer);

    std::string subexec(std::string path, Request* req, System* router = NULL, Client* client = NULL);

    // std::string exec2(std::string path, Request* req, System* router = NULL, Client* client = NULL);

    // std::string exec3(std::string path, Request* req, System* router = NULL, Client* client = NULL);

    int registerSymbol(std::string symbol);

    void dumpSymbols();

    void dump();

    void system(std::string path, RouteFunction fn);

    void secure(std::string path, RouteFunction fn = NULL);

    bool secured(std::string path);

    void resource(std::string path, ResourceFunction fn);

    Response* resource(System* system, Request* req, jericho::Session* sesh);

    std::string exec(RouteProtocol protocol, std::string path, Request* req, System* router = NULL, Client* client = NULL);

    // std::string execNode(RouteProtocol protocol, std::string path, Request* req, System* router = NULL, Client* client = NULL);

    Route* route(std::string path) {
        for (auto r : _routes) {
            if (r->path == path) return r;
        }
        return NULL;
    }

    // void ipath(std::string path, std::string ipath) {
    //     _router->ipath(path, ipath);
    // }

    // std::string ipath(std::string path) {
    //     return _router->ipath(path);
    // }
};

#endif