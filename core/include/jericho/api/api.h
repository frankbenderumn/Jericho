#ifndef CELERITY_API_API_H_
#define CELERITY_API_API_H_

#include <string>
#include <unordered_map>

#include "router/router.h"
#include "server/defs.h"
#include "api/api_helper.h"

API(Federate, ARGS{})
    ClusterNode* boss = router->cluster()->boss();
    boss->federate(router, client, "/federate-local", 5, 5);
    return "TICKET";
}


#endif