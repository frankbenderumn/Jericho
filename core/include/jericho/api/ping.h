#ifndef API_PING_H_
#define API_PING_H_

#include "api/api.h"

API(PingAll, {})
    if (router == NULL || client == NULL) {
        return JsonResponse::error(500, "Cluster not in scope of api call");
    }

    router->ws_send("Pinging all");

    // for naive approach testing (no predefined topology)
    std::vector<std::string> set = {
        {"http://127.0.0.1:8081/ping"},
        {"http://127.0.0.1:8082/ping"},
        {"http://127.0.0.1:8083/ping"},
        {"http://127.0.0.1:8084/ping"},
        {"http://127.0.0.1:8085/ping"},
        {"http://127.0.0.1:8086/ping"},
        {"http://127.0.0.1:8087/ping"},
        {"http://127.0.0.1:8088/ping"},
        {"http://127.0.0.1:8089/ping"},
        {"http://127.0.0.1:8090/ping"}
    };

    std::string result = router->bifrost()->broadcast(set, "ping");

    return result;
}

API(Ping, {})
    return "{\"live\": \"true\", \"response\": \"hello from " + router->bifrost()->host() + ":" + router->bifrost()->port() + "\"}";
}

#endif