#ifndef JERICHO_ROUTER_H_
#define JERICHO_ROUTER_H_

#include <string>

#include "server/response.h"
#include "server/client.h"
#include "server/resource.h"

namespace parser2 {
    
    void parse(Client* client, Client** clients);

}

#endif