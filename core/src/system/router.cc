#include "system/router.h"
#include "server/request.h"

bool Router::hasFunction(std::string path) {
    if (_routeFunctions.find(path) != _routeFunctions.end()) return true;
    // if (_systemFunctions.find(path) != _systemFunctions.end()) return true;
    if (_httpFunctions.find(path) != _httpFunctions.end()) return true;
    return false;
}

bool Router::hasRoute(std::string route) {
    if (_routeTable.find(route) != _routeTable.end()) return true;
    return false;
}

bool Router::hasSymbol(std::string symbol) {
    if (_symbols.find(symbol) != _symbols.end()) return true;
    return false;
}

bool Router::hasNode(std::string path) {
    if (_nodes.find(path) != _nodes.end()) return true;
    return false;
}

void Router::registerRoute(std::string routeName, Route* route) {
    if (!hasRoute(routeName)) {
        _routeTable[routeName] = route;
    }
}

void Router::securePath(std::string path, RouteFunction fn) {
    Route* route = new Route(ROUTE_SECURE, path, fn);
    route->secure = true;
    _routes.push_back(route);
    _securePaths.push_back(path);
    _routeProtocols[path] = ROUTE_SECURE;
}

const std::vector<std::string>& Router::securePaths() const {
    return _securePaths;
}

RouteProtocol Router::protocol(std::string path) {
    if (_routeProtocols.find(path) != _routeProtocols.end()) {
        return _routeProtocols[path];
    }
    return ROUTE_NULL;
}

void Router::bind(RouteProtocol protocol, std::string path, RouteFunction function, std::vector<ContractFunction> pres, std::vector<ContractFunction> posts) {
    Route* route = new Route(protocol, path, function);
    route->pre = pres;
    route->post = posts;
    _routes.push_back(route);
    _routeProtocols[path] = protocol;
    _routeTable[path] = route;
    _test[path] = "get some";
    switch (protocol) {
        case ROUTE_API:
            _routeFunctions[path] = function;
            break;
        case ROUTE_HTTP:
            _httpFunctions[path] = function;
            break;
        case ROUTE_IRIS:
            _httpFunctions[path] = function;
            break;
        case ROUTE_SYSTEM:
            _httpFunctions[path] = function;  
            break;      
    }
}

void Router::ipath(std::string path, std::string ipath) {
    _ipaths[path] = ipath;
    _routeProtocols[path] = ROUTE_IRIS;
}

std::string Router::ipath(std::string path) {
    return _ipaths[path];
}

bool Router::contains(std::string path) {
    return prizm::contains_key(_ipaths, path);
}
// std::unordered_map<std::string, std::string>& ipaths() { return _ipaths; }

void Router::bindSystem(std::string path, RouteFunction function, RouteProtocol protocol) {
    _routeProtocols[path] = protocol;
    _routeFunctions[path] = function;
}

void Router::bindNode(std::string path, Message* buffer) {
    _routeProtocols[path] = ROUTE_CLUSTER;
    _nodes[path] = buffer;
}

// sort based on priority for static. Need more advanced approach for cms ex: /user/:id/posts
std::string Router::subexec(std::string path, Request* req, System* router, Client* client) {
    bool found = false;
    for (auto route : _routes) {
        if (route->path == path) { 
            return route->exec(req, router, client, NULL);
        }
    }

    return JsonResponse::error(500, "Router::exec: Route not found!");
}

void Router::resource(std::string path, ResourceFunction fn) {
    Route* route = new Route(ROUTE_RESOURCE, path, NULL);
    route->resource = fn;
    _routes.push_back(route);
    _routeProtocols[path] = ROUTE_RESOURCE;
    _routeTable[path] = route;
    _test[path] = "get some";
}

Response* Router::resource(System* system, Request* request, jericho::Session* sesh) {
    for (auto route : _routes) {
        if (route->path == request->path) {
            if (route->resource) {
                return route->resource(system, request, sesh);
            } else {
                return NULL;
            }
        }
    }
    return NULL;
}


// std::string Router::exec3(std::string path, std::unordered_map<std::string, std::string> args, System* router, Client* client) {
//     if (!prizm::contains_key(_test, path)) {
//         return JsonResponse::error(500, "Unregistered path detected for path: " + path);
//     }

//     return _test[path];

//     return JsonResponse::error(500, "Invalid protocol provided");
// }

// std::string Router::exec2(std::string path, std::unordered_map<std::string, std::string> args, System* router, Client* client) {
//     if (!hasFunction(path)) {
//         return JsonResponse::error(500, "Function does not exist for path: " + path);
//     }
//     switch (_routeProtocols[path]) {
//         case ROUTE_API:
//             return _routeFunctions[path](args, router, client, NULL);
//         case ROUTE_HTTP:
//             return _httpFunctions[path](args, router, client, NULL);
//         case ROUTE_SYSTEM:
//             return _routeFunctions[path](args, router, client, NULL);
//         case ROUTE_RAW:
//             return _routeFunctions[path](args, router, client, NULL);
//         default:
//             break;
//     }

//     return JsonResponse::error(500, "Invalid protocol provided");
// }

int Router::registerSymbol(std::string symbol) {
    if (!hasSymbol(symbol)) {
        _symbols[symbol] = ++wc_symbol_id;
        return wc_symbol_id;
    }
    return _symbols[symbol];
}

void Router::dumpSymbols() {
    for (auto sym : _symbols) {
        printf("sym: %s %i\n", sym.first.c_str(), sym.second);
    }
}

void Router::dump() {
    // for (auto route : this->_routeTable) {
    //     route.second->dump();
    // }
    for (auto route : this->_routes) {
        route->dump();
    }
}

void Router::system(std::string path, RouteFunction fn) {
    Route* route = new Route(ROUTE_SYSTEM, path, fn);
    _routes.push_back(route);
    _routeProtocols[path] = ROUTE_SYSTEM;
    _routeFunctions[path] = fn;
    _routeTable[path] = route;
    _test[path] = "get some";
}   

bool Router::secured(std::string path) {
    // this->bindSystem(path, fn);
    for (auto route : this->_routes) {
        if (route->path == path) {
            if (route->secure) {
                return true;
            }
            return false;
        }
    }
    return false;
}

void Router::secure(std::string path, RouteFunction fn) {
    this->securePath(path, fn);
}

std::string Router::exec(RouteProtocol protocol, std::string path, Request* req, System* router, Client* client) {
    // YEL("System::exec: Executing route %s...n", path.c_str());
    PLOG(LSERVER, "Executing route \033[0;35m%s\033[0m", path.c_str());
    return this->subexec(path, req, router, client);
}

// std::string Router::execNode(RouteProtocol protocol, std::string path, std::unordered_map<std::string, std::string> args, System* router, Client* client) {
//     if (client == NULL) {
//         return JsonResponse::error(500, "No client to serve!");
//     }

//     return JsonResponse::error(500, "Exec node deprecated");
// }