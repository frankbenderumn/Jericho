#ifndef CELERITY_EXTERNAL_API_H_
#define CELERITY_EXTERNAL_API_H_

#include "api/string_utils.h"
#include <unordered_map>
#include <map>

struct JsonResponse {
    static std::string error(int code, std::string message) {
        return "{\"status\": \""+std::to_string(code)+"\", \"error\": \""+message+"\"}";
    }
};

template<typename K, typename V>
std::vector<K> keys(std::map<K, V> map) {
    std::vector<K> result = {};
    for (auto kv : map) {
        result.push_back(kv.first);
    }
    return result;
}

template<typename K, typename V>
std::vector<V> values(std::map<K, V> map) {
    std::vector<V> result = {};
    for (auto kv : map) {
        result.push_back(kv.second);
    }
    return result;
}


struct Route {
    std::string url;
    std::string path;
    std::string protocol;
    std::vector<std::string> wildcards;
    std::vector<std::string> args;
    std::map<std::string, std::string> kvs;
    std::string signature;
};

typedef std::string (*RouteFunction)(std::vector<std::string>);

static int wc_symbol_id = -1;

class RouteRegistry {
    std::unordered_map<std::string, int> _symbols;
    std::unordered_map<std::string, Route*> _routeTable;
    std::unordered_map<std::string, RouteFunction> _routeFunctions;

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

    void registerRoute(std::string routeName, Route* route) {
        if (!hasRoute(routeName)) {
            _routeTable[routeName] = route;
        }
    }

    void bind(std::string path, RouteFunction function) {
        _routeFunctions[path] = function;
    }

    std::string exec(std::string path, std::vector<std::string> args) {
        if (!hasFunction(path)) {
            return JsonResponse::error(500, "Function does not exist for path: " + path);
        }
        return _routeFunctions[path](args);
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

  public:
    Router() {
        _registry = new RouteRegistry;
    }

    ~Router() {
        delete _registry;
    }

    void bind(std::string path, RouteFunction function) {
        _registry->bind(path, function);
    }

    std::string exec(std::string path, std::vector<std::string> args) {
        return _registry->exec(path, args);
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
        printf("some bullshit\n");
        std::vector<std::string> dirs = tokenize(path, '/');
        dirs[0] = "/" + dirs[0];
        std::vector<std::string> wildcards = {};
        std::string args_str = "";
        std::string path2 = "";
        if (path.find("?") != std::string::npos) {
            BRED("? found\n");
            std::string::size_type const p(path.find_first_of('?'));
            path2 = path.substr(0, p);
            args_str = path.substr(p+1, path.size());
        }
        printf("iterating directories\n");
        BRED("Iterating directories\n");
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
        // request->args = args;
        // request->wildcards = wildcards;
        BRED("Assignments to route done\n");
        for (auto w : wildcards) {
            printf("wc: %s\n", w.c_str());
        }
        for (auto arg : args) {
            printf("arg: %s\n", arg.c_str());
        }
        if(signature.size() != 0) {
            signature.pop_back();
        }
        request->signature = signature;
        printf("signature: %s\n", signature.c_str());
    }
};

#endif