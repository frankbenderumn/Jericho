#ifndef JERICHO_ROUTER_H_
#define JERICHO_ROUTER_H_

#include <string>

#include "server/response.h"
#include "server/client.h"
#include "server/resource.h"

namespace parser2 {

    // std::string parse(std::string uri) {

    //     if (uri == "/user/$id") {

    //     } else if (uri == "/stock/$id") {

    //     } else if (uri == "/transaction/$id") {

    //     } else if (uri == "/ping") {

    //     } else if (uri == "/pong")

    // }

    /** TODO: switch to large scale response parse (send_response(code, socket)) */ 
    void parse(Client* client, Client** clients);

}

#endif