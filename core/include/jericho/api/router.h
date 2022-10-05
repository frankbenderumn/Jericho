#ifndef CELERITY_EXTERNAL_API_H_
#define CELERITY_EXTERNAL_API_H_

#include <pthread.h>

#include <set>
#include <unordered_map>
#include <map>
#include <queue>

#include "api/string_utils.h"
#include "server/defs.h"
#include "server/fetch.h"

using namespace Jericho;

typedef void (*WorkerThread)(void*);

struct JsonResponse {
    static std::string error(int code, std::string message) {
        return "{\"status\": \""+std::to_string(code)+"\", \"error\": \""+message+"\"}";
    }
    static std::string success(int code, std::string message) {
        return "{\"status\": \""+std::to_string(code)+"\", \"success\": \""+message+"\"}";
    }
};

template<typename T>
bool contains(std::vector<T> vec, T el) {
    for (auto v : vec) {
        if (v == el) return true;
    }
    return false;
}

template<typename K, typename V>
bool containsKey(std::unordered_map<K, V> map, K el) {
    for (auto m : map) {
        if (m.first == el) return true;
    }
    return false;
}

template<typename K, typename V>
bool containsVal(std::unordered_map<K, V> map, V el) {
    for (auto m : map) {
        if (m.second == el) return true;
    }
    return false;
}

template<typename K, typename V>
bool containsItem(std::unordered_map<K, V> map, K el, V el2) {
    for (auto m : map) {
        if (m.first == el && m.second == el2) return true;
    }
    return false;
}

template<typename T>
void print(std::vector<T> vec) {
    for (auto v : vec) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;
}

template<typename T>
void print(std::set<T> vec) {
    for (auto v : vec) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;
}

template<typename K, typename V>
std::set<K> keys(std::map<K, V> map) {
    std::set<K> result = {};
    for (auto kv : map) {
        result.insert(kv.first);
    }
    return result;
}

template<typename K, typename V>
std::set<V> values(std::map<K, V> map) {
    std::set<V> result = {};
    for (auto kv : map) {
        result.insert(kv.second);
    }
    return result;
}

template<typename K, typename V>
std::set<K> keys(std::unordered_map<K, V> map) {
    std::set<K> result = {};
    for (auto kv : map) {
        result.insert(kv.first);
    }
    return result;
}

template<typename K, typename V>
std::set<V> values(std::unordered_map<K, V> map) {
    std::set<V> result = {};
    for (auto kv : map) {
        result.insert(kv.second);
    }
    return result;
}

template<typename T>
bool subset(std::set<T> set_one, std::set<T> set_two) {
    return (std::includes(set_two.begin(), set_two.end(),
            set_one.begin(), set_one.end()));
}

struct Route {
    std::string url;
    std::string path;
    std::string protocol;
    std::vector<std::string> wildcards;
    std::vector<std::string> args;
    std::unordered_map<std::string, std::string> kvs;
    std::string signature;
};

typedef std::string (*RouteFunction)(std::unordered_map<std::string, std::string>);
typedef std::string (*ClusterFunction)(ThreadPool* tpool, MessageQueue* mq, void* (*worker)(void*), std::string message);

static int wc_symbol_id = -1;

enum RouteProtocol {
    ROUTE_NULL,
    ROUTE_HTTP,
    ROUTE_API,
    ROUTE_CLUSTER
};

struct ClusterNode {
    std::string name;
};

struct MessageBuffer {
    MessageQueue* mq;
    std::string sent;
    std::string hostname;
    std::string port;
    std::string path;
    std::string received;
};

class RouteRegistry {
    std::unordered_map<std::string, int> _symbols;
    std::unordered_map<std::string, RouteProtocol> _routeProtocols;
    std::unordered_map<std::string, Route*> _routeTable;
    std::unordered_map<std::string, RouteFunction> _routeFunctions;
    std::unordered_map<std::string, RouteFunction> _httpFunctions;
    std::unordered_map<std::string, MessageBuffer*> _nodes;
    std::vector<std::string> _securePaths;

  public:
    bool hasFunction(std::string path) {
        if (_routeFunctions.find(path) != _routeFunctions.end()) return true;
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

    void bindNode(std::string path, MessageBuffer* buffer) {
        _routeProtocols[path] = ROUTE_CLUSTER;
        _nodes[path] = buffer;
    }

    std::string exec(std::string path, std::unordered_map<std::string, std::string> args) {
        if (!hasFunction(path)) {
            return JsonResponse::error(500, "Function does not exist for path: " + path);
        }
        if (_routeProtocols[path] == ROUTE_API) {
            return _routeFunctions[path](args);
        } else if (_routeProtocols[path] == ROUTE_HTTP) {
            return _httpFunctions[path](args);
        }
        return JsonResponse::error(500, "Invalid protocol provided");
    }

    std::string execNode(std::string path, ThreadPool* tpool, WorkerThread worker, std::unordered_map<std::string, std::string> args) {
        if(!hasNode(path)) {
            return JsonResponse::error(500, "Node not bound to path");
        } else {
            MessageBuffer* buf = _nodes[path];
            thread_pool_add(tpool, worker, (void*)buf);
            return buf->received;
        }
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

class Router {
    RouteRegistry* _registry = nullptr;
    ThreadPool* _tpool;
    WorkerThread _worker;

  public:
    Router(ThreadPool* tpool, WorkerThread worker) {
        _registry = new RouteRegistry;
        _worker = worker;
        _tpool = tpool;
    }

    ~Router() {
        delete _registry;
    }

    void bind(RouteProtocol protocol, std::string path, RouteFunction function) {
        _registry->bind(protocol, path, function);
    }

    void bindNode(std::string path, MessageBuffer* buffer) {
        _registry->bindNode(path, buffer);
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

    // void bindHttp(std::string path, RouteFunction function) {
    //     _registry->bindHttp(path, function);
    // }

    std::string exec(std::string path, std::unordered_map<std::string, std::string> args) {
        return _registry->exec(path, args);
    }

    std::string execNode(std::string path, std::unordered_map<std::string, std::string> args) {
        return _registry->execNode(path, _tpool, _worker, args);
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