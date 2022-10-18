#ifndef JERICHO_API_API_HELPER_H_
#define JERICHO_API_API_HELPER_H_

#include <regex>
#include <string>
#include <set>
#include <unordered_map>
#include <utility>

#include "server/defs.h"
#include "server/response.h"

#define OPT std::pair<std::string, std::string>
typedef std::unordered_map<std::string, std::string> Args;
typedef std::regex_iterator<std::string::iterator> RegexIter;

bool apiValidate(std::string substr, std::regex rgx) {
    RegexIter it(substr.begin(), substr.end(), rgx);
    RegexIter end;
    if (std::distance(it, end) != 0) {
        return true;
    }
    return false;
}

std::string pipe(std::string command) {
    std::string result;
    FILE* fp;
    int status = -1;

    fp = popen(command.c_str(), "r");
    if (fp == NULL) { return "PIPE ERROR"; }

    int someLen = 0;
    int lineLen = 10000;
    char *line;
    line = (char *)malloc(sizeof(char) * lineLen);
    if (line == NULL) {
        return "PIPE ERROR";
    }
    while (fgets(line, lineLen, fp)) {
        BMAG("LINE: %s\n", line);
        result += std::string(line);
        someLen += lineLen;
        if (someLen != lineLen) {
            line = (char *)realloc(line, sizeof(char) * someLen);
        }
        if (line == NULL) {
            return "PIPE ERROR";
        }
    }

    printf("Size of line is: %i\n", (int)result.size());
    BGRE("LINE: %s\n", result.c_str());
    free(line);

    status = pclose(fp);
    if (status == -1) {
        return "PIPE ERROR";
    }

    return result;
}

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

#define API(x, y) std::string api##x(Args args, \
                                    Router* router = NULL, \
                                    Client* client = NULL) { \
    if (router == NULL) \
        return JsonResponse::error(404, "Client does not exist!"); \
    if (client == NULL) \
        return JsonResponse::error(404, "Router does not exist!"); \
    if (!containsKey(TOKEN_LIST, args["token"])) \
        return JsonResponse::error(404, "Invalid token provided!"); \
    if (!subset(y, keys(args))) \
        return JsonResponse::error(404, "Invalid arguments");

#endif