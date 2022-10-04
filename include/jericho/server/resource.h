#ifndef JERICHO_SERVER_RESOURCE_H_
#define JERICHO_SERVER_RESOURCE_H_

// inherits from endpoint?

#include <string>
#include "util/file_system.hpp"
#include "server/response.h"

namespace resource {
    void error(Client* client, const std::string& path);

    void serve_cxx(Client* conn, Client** clients, const char* path);

    void serve_dist(Client* conn, Client** clients, const char* message);

}

#endif