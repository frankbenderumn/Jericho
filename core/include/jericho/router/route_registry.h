#ifndef JERICHO_ROUTER_ROUTE_REGISTRY_H_
#define JERICHO_ROUTER_ROUTE_REGISTRY_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "server/response.h"
#include "server/defs.h"

#include "router/route_functions.h"

class MessageBuffer;
class Router;

struct Route {
    std::string url;
    std::string path;
    std::string protocol;
    std::vector<std::string> wildcards;
    std::vector<std::string> args;
    std::unordered_map<std::string, std::string> kvs;
    std::string signature;
};

static int wc_symbol_id = -1;

enum RouteProtocol {
    ROUTE_NULL,
    ROUTE_HTTP,
    ROUTE_API,
    ROUTE_CLUSTER,
    ROUTE_SYSTEM,
    ROUTE_DB,
    ROUTE_RAW,
    ROUTE_JSON,
    ROUTE_XML,
    ROUTE_YAML,
    ROUTE_CSV
};

static int MESSAGE_ID = -1;

class RouteRegistry {
    std::unordered_map<std::string, int> _symbols;
    std::unordered_map<std::string, RouteProtocol> _routeProtocols;
    std::unordered_map<std::string, Route*> _routeTable;
    std::unordered_map<std::string, RouteFunction> _routeFunctions;
    std::unordered_map<std::string, RouteFunction> _httpFunctions;
    std::unordered_map<std::string, SystemFunction> _systemFunctions;
    std::unordered_map<std::string, MessageBuffer*> _nodes;
    std::vector<std::string> _securePaths;

  public:
    bool hasFunction(std::string path) {
        if (_routeFunctions.find(path) != _routeFunctions.end()) return true;
        if (_systemFunctions.find(path) != _systemFunctions.end()) return true;
        if (_httpFunctions.find(path) != _httpFunctions.end()) return true;
        return false;
    }

    bool hasRoute(std::string route) {
        if (_routeTable.find(route) != _routeTable.end()) return true;
        return false;
    }

    bool hasSymbol(std::string symbol) {
        if (_symbols.find(symbol) != _symbols.end()) return true;
        return false;
    }

    bool hasNode(std::string path) {
        if (_nodes.find(path) != _nodes.end()) return true;
        return false;
    }

    void registerRoute(std::string routeName, Route* route) {
        if (!hasRoute(routeName)) {
            _routeTable[routeName] = route;
        }
    }

    void securePath(std::string path) {
        _securePaths.push_back(path);
    }

    const std::vector<std::string>& securePaths() const {
        return _securePaths;
    }

    RouteProtocol protocol(std::string path) {
        if (_routeProtocols.find(path) != _routeProtocols.end()) {
            return _routeProtocols[path];
        }
        return ROUTE_NULL;
    }

    void bind(RouteProtocol protocol, std::string path, RouteFunction function) {
        _routeProtocols[path] = protocol;
        if (protocol == ROUTE_API) {
            _routeFunctions[path] = function;
        } else if (protocol == ROUTE_HTTP) {
            _httpFunctions[path] = function;
        }
    }

    void bindSystem(std::string path, SystemFunction function, RouteProtocol protocol = ROUTE_SYSTEM) {
        _routeProtocols[path] = protocol;
        _systemFunctions[path] = function;
    }

    void bindNode(std::string path, MessageBuffer* buffer) {
        _routeProtocols[path] = ROUTE_CLUSTER;
        _nodes[path] = buffer;
    }

    std::string exec(std::string path, std::unordered_map<std::string, std::string> args, Router* router = NULL, Client* client = NULL) {
        if (!hasFunction(path)) {
            return JsonResponse::error(500, "Function does not exist for path: " + path);
        }

        if (_routeProtocols[path] == ROUTE_API) {
            return _routeFunctions[path](args);
        } else if (_routeProtocols[path] == ROUTE_HTTP) {
            return _httpFunctions[path](args);
        } else if (_routeProtocols[path] == ROUTE_SYSTEM) {
            return _systemFunctions[path](args, router, client, NULL);
        } else if (_routeProtocols[path] == ROUTE_RAW) {
            return _systemFunctions[path](args, router, client, NULL);
        }

        return JsonResponse::error(500, "Invalid protocol provided");
    }

    int registerSymbol(std::string symbol) {
        if (!hasSymbol(symbol)) {
            _symbols[symbol] = ++wc_symbol_id;
            return wc_symbol_id;
        }
        return _symbols[symbol];
    }

    void dumpSymbols() {
        for (auto sym : _symbols) {
            printf("sym: %s %i\n", sym.first.c_str(), sym.second);
        }
    }
};

#endif