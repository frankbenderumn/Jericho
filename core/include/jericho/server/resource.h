#ifndef JERICHO_SERVER_RESOURCE_H_
#define JERICHO_SERVER_RESOURCE_H_

// inherits from endpoint?

#include <string>
#include "util/file_system.hpp"
#include "server/defs.h"
// #include "server/response.h"
// #include "system/router.h"

class Router;
class System;

namespace resource {
    void error(Router* router, Client* client, const std::string& path);

    void serve_cxx(System* router, Client* conn, Client** clients, const char* path);

    void serve_raw(Client* conn, Client** clients, const char* message);

    void serve_http(Client* conn, Client** clients, const char* message, std::string type = "text/html");
}

#endif