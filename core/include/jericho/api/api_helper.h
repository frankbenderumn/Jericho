#ifndef JERICHO_API_API_HELPER_H_
#define JERICHO_API_API_HELPER_H_

#include <regex>
#include <string>
#include <set>
#include <unordered_map>
#include <utility>

#include "server/defs.h"
#include "server/response.h"
#include "serializer/serializer.h"

#define OPT std::pair<std::string, std::string>
typedef std::unordered_map<std::string, std::string> Args;
typedef std::regex_iterator<std::string::iterator> RegexIter;
#define SYS_CALL(x) std::string x(Request* req, System* router = NULL, Client* client = NULL)
#define API_ARG(args, name) (containsKey(req->args, name)) ? req->args[name] : "undefined";

#define REQUEST_INFO std::string hostname = (containsKey(req->args, std::string("Host"))) ? req->args["Host"] : "undefined"; \
std::string host; std::string port; \
if (hostname != "undefined") { \
    std::vector<std::string> vec = tokenize(hostname, ':'); \
    host = vec[0]; \
    port = vec[1]; \
}

bool apiValidate(std::string substr, std::regex rgx);

std::string pipe(std::string command);

const std::unordered_map<std::string, int> TOKEN_LIST = {
    {"kalmoru", 0},
    {"kobutcha", 0}
};

template <typename K, typename V>
bool contains(std::unordered_map<K, V> map, K key) {
    if (map.find(key) != map.end()) return true;
    return false;
}

#define ARGS std::set<std::string>

#define API(x, y) std::string api##x(Request* req, \
                                    System* router = NULL, \
                                    Client* client = NULL, \
                                    MessageBroker* broker = NULL) { \
    if (router == NULL) \
        return JsonResponse::error(404, "Client does not exist!"); \
    if (client == NULL) \
        return JsonResponse::error(404, "System does not exist!");
    // if (!containsKey(TOKEN_LIST, args["token"])) \
    //     return JsonResponse::error(404, "Invalid token provided!"); \
    // if (!subset(y, keys(args))) \
    //     return JsonResponse::error(404, "Invalid arguments");

#endif