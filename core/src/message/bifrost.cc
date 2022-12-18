#include "message/bifrost.h"
#include "router/router.h"
#include "server/request.h"
#include "server/fetch.h"
#include "celerity/celerity.h"
#include "message/message_buffer.h"
#include "message/message_broker.h"
#include "api/api_helper.h"
#include "message/callback.h"
#include "cluster/cluster_node.h"

// for client only use, may need to make thread pool more robust to handle behavior that doesn't require incoming connections

void Bifrost::burst(Router* router) {
    MAG("Bifrost::serve: Start...\n");
    std::vector<BifrostBurst*> bursts = router->cluster()->boss()->bursts();
    MAG("Bifrost::serve: Burst size: %li\n", bursts.size());
    if (bursts.size() > 0) {
        for (auto burst : bursts) {
            MessageBroker* broker = burst->broker;
            if (broker->hasMessages()) {
                MAG("Bifrost::serve: FIRING ASYNC CALL - EPOCH: %i\n", broker->epoch());
                MAG("Bifrost::serve: Client url is: %s\n", burst->url.c_str());
                MAG("Bifrost::serve: Compiling callback...\n");
                std::deque<MessageBuffer*> bufs = broker->response(burst->url);
                broker->stash(bufs);
                std::string response = broker->callback()(router, burst->client, bufs, broker->callbackType(), broker->args());
                MAG("Bifrost::serve: Response: %s\n", response.c_str());    
                if (broker->epoch() == broker->epochs()) {
                    if (isHTTP(response)) {
                        MAG("Bifrost::serve: Serve raw\n");
                        // resource::serve_raw(burst->client, clients, response.c_str());
                    } else {
                        MAG("Bifrost::serve: Serve HTTP\n");
                        // resource::serve_http(burst->client, clients, response.c_str());
                    }
                    // client->promised = false;
                    router->cluster()->boss()->serveBroker(burst->url, broker);
                    MAG("Bifrost::serve: End poll true\n");
                    // drop_client(client, clients); // this segfaults buffer size of 500000 but not 4095 (fixed...?)
                }
            }
        }
    }
}

bool Bifrost::poll(Router* router, std::string url, Client* client, Client** clients) {
    // MAG("Bifrost::poll: They see me pollin, they hatin\n");
    MessageBroker* broker = router->cluster()->boss()->poll(url);
    if (broker != NULL) {
        MAG("Bifrost::poll: FIRING ASYNC CALL - EPOCH: %i\n", broker->epoch());
        MAG("Bifrost::poll: Client url is: %s\n", url.c_str());
        MAG("Bifrost::poll: Compiling callback...\n");
        std::deque<MessageBuffer*> bufs = broker->response(url);
        broker->stash(bufs);
        std::string response = broker->callback()(router, client, bufs, broker->callbackType(), broker->args());
        MAG("Bifrost::poll: Response: %s\n", response.c_str());    
        MAG("Bifrost::poll: Epoch: %i, Epochs: %i\n", broker->epoch(), broker->epochs());    
        if (broker->epoch() - 1 == broker->epochs()) {
            if (isHTTP(response)) {
                MAG("Bifrost::poll: Serve raw\n");
                resource::serve_raw(client, clients, response.c_str());
            } else {
                MAG("Bifrost::poll: Serve HTTP\n");
                resource::serve_http(client, clients, response.c_str());
            }
            // client->promised = false;
            router->cluster()->boss()->serveBroker(url, broker);
            MAG("Bifrost::poll: End poll true\n");
            return true;
            // drop_client(client, clients); // this segfaults buffer size of 500000 but not 4095 (fixed...?)
        }
    }
    // MAG("Bifrost::poll: End poll false\n");
    return false;
}