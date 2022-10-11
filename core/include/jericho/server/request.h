#ifndef JERICHO_SERVER_REQUEST_H_
#define JERICHO_SERVER_REQUEST_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

#include "util/string_utils.h"

#include "prizm/prizm.h"
#include "server/resource.h"
#include "server/client.h"

struct Request {
    std::string content;
    std::unordered_map<std::string, std::string> headers;
    std::string method;
    std::string path;
    std::string request;
    std::unordered_map<std::string, std::string> args;
    std::string signature;
};

bool is_valid_request(Client* client);

int parse_request(Client* client, Request* request);

void print_request(Request* request);

bool isHTTP(std::string request);

#endif