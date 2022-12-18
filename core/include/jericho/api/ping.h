#ifndef API_PING_H_
#define API_PING_H_

#include "api/api.h"

API(Ping, {})
    std::string localHost = router->cluster()->boss()->host();
    std::string localPort = router->cluster()->boss()->port();
    BMAG("Ping: Start on %s:%s\n", localHost.c_str(), localPort.c_str());
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    if (broker != NULL) {
        // broker->refresh();
        MessageBuffer* buf = new MessageBuffer;
        BWHI("apiPing: CLIENT URL IS: %s:%s\n", host.c_str(), port.c_str());
        BYEL("Ping: Content: %s\n", content.c_str());
        buf->hostname = host;
        buf->port = port;
        buf->dir = "./public/cluster/" + port;
        buf->broker = broker;
        buf->sent = "Ping from 8080...";
        buf->fromPort = "8080";
        buf->path = "/ping-local";
        buf->ticket = ++TICKET_ID;
        buf->client = host + ":" + port;
        BBLU("apiPing: Phase %i result: %s\n", broker->epoch(), content.c_str());
        router->cluster()->boss()->send(router, buf->client, "/ping-local", buf);
        return "TICKET";
    }
    return JsonResponse::success(200, "Ping from ... (need to make this info global)\n");
}

API(PingOne, {})
    if (!contains(TOKEN_LIST, args["token"])) {
        return JsonResponse::error(404, "Invalid token provided");
    }

    if (router == NULL || client == NULL) {
        return JsonResponse::error(500, "Cluster not in scope of api call");
    }

    if (!containsKey(args, std::string("port"))) {
        return JsonResponse::error(404, "Port needs to be specified");
    }

    ClusterNode* node = router->cluster()->boss()->getEdge("127.0.0.1", args["port"]);


    if (node != nullptr) {
        BYEL("Selected Node, Host: %s, Port: %s\n", node->host().c_str(), node->port().c_str());
        router->cluster()->pingOne(router, client->url, node);
    } else {
        return JsonResponse::error(404, "Host and port are not part of cluster. Spawn instance first");
    }

    return "TICKET";
}

API(PingAll, {})
    // if (!contains(TOKEN_LIST, args["token"])) {
    //     return JsonResponse::error(404, "Invalid token provided");
    // }

    if (router == NULL || client == NULL) {
        return JsonResponse::error(500, "Cluster not in scope of api call");
    }

    router->ws_send("Get some");

    // for naive approach testing (no predefined topology)
    std::vector<std::pair<std::string, std::string>> set = {
        {"127.0.0.1", "8081"},
        {"127.0.0.1", "8082"},
        {"127.0.0.1", "8083"},
        {"127.0.0.1", "8084"},
        {"127.0.0.1", "8085"},
        {"127.0.0.1", "8086"},
        {"127.0.0.1", "8087"},
        {"127.0.0.1", "8088"},
        {"127.0.0.1", "8089"},
        {"127.0.0.1", "8090"}
    };

    BCYA("PINGING ALL...\n");
    // for predefined topology
    // router->cluster()->pingAll(router, client);

    // for naive topology testing (clients come and go)
    router->cluster()->pingSet(router, client->url, set);

    return "TICKET";
}

API(Echo, {})
    std::string localHost = router->cluster()->boss()->host();
    std::string localPort = router->cluster()->boss()->port();
    BMAG("PingLocal: Start on %s:%s\n", localHost.c_str(), localPort.c_str());
    std::string host = router->cluster()->boss()->host();
    std::string port = router->cluster()->boss()->port();
    std::string msg = "Echo from " + host + ":" + port;
    return "{\"live\": true, \"response\": \""+msg+"\"}";
}

API(PingLocal, {})
    REQUEST_INFO
    std::string msg = "Hello from " + host + ":" + port;
    std::string name = router->cluster()->boss()->url();
    MessageBuffer* buf = new MessageBuffer(name, hostname + "/echo", router->cluster()->boss()->dir(), msg);
    broker = new MessageBroker(BROKER_FIFO, single_callback);
    buf->broker = broker;
    // buf->dump();
    router->cluster()->boss()->brokers()[client->url].push_back(broker);
    router->cluster()->boss()->send(router, client->url, std::string("/ping"), buf);
    return "{\"live\": true, \"response\": \""+msg+"\"}";
}

#endif