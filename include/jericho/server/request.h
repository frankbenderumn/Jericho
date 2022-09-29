#ifndef JERICHO_SERVER_REQUEST_H_
#define JERICHO_SERVER_REQUEST_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "prizm/prizm.h"
#include "server/resource.h"
#include "server/client.h"
#include "api/string_utils.h"

struct Request {
    std::string content;
    std::unordered_map<std::string, std::string> headers;
    std::string method;
    std::string path;
    std::string request;
    std::vector<std::string> args;
};

bool is_valid_request(Client* client);

int parse_request(Client* client, Request* request);

void print_request(Request* request);

#endif