#ifndef API_FEDERATED_H_
#define API_FEDERATE_H_

#include "api/api.h"
#include "api/ping.h"
#include "router/gui.h"

API(Federate, {})
    ClusterNode* boss = router->cluster()->boss();
    boss->federate(router, client->url, "/federate-local", 5, 5);
    return "TICKET";
}

API(FederateLocal, {})
    if (router == NULL) {
        return JsonResponse::error(500, "Cluster not in scope of api call");
    }

    std::string host = router->cluster()->boss()->host();
    std::string port = router->cluster()->boss()->port();

    if (containsKey(args, std::string("content"))) {
        return "Federating from " + host + ":" + port + ". Received message " + args["content"];
    } else {
        return "Federating from " + host + ":" + port + ". No message sent.";
    }
}

API(ServeModel, {})
    std::string bytes = Jericho::FileSystem::readBinary("./py/scripts/torch.pt");
    return bytes;
}

API(Dispatch, {})
    std::string localHost = router->cluster()->boss()->host();
    std::string localPort = router->cluster()->boss()->port();
    BMAG("Dispatch: Start on %s:%s\n", localHost.c_str(), localPort.c_str());
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    std::string agg = router->federator()->local()->dispatch(content);
    BYEL("Dispatch: Content: %s\n", content.c_str());
    if (broker != NULL) {
        MessageBuffer* buf = new MessageBuffer;
        buf->hostname = host;
        buf->port = port;
        buf->dir = "./public/frontend";
        buf->broker = broker;
        buf->sent = agg;
        buf->fromPort = "8080";
        buf->path = "/fed-node";
        buf->ticket = ++TICKET_ID;
        buf->client = host + ":" + port;
        router->cluster()->boss()->send(router, client->url, std::string("/ping"), buf);
        return JsonResponse::success(200, "DISPATCHED\n");
    }
    BMAG("Dispatch: End\n");
    return JsonResponse::success(200, "DISPATCHED\n");
}

API(DispatchJoin, {})
    REQUEST_INFO
    std::string localHost = router->cluster()->boss()->host();
    std::string localPort = router->cluster()->boss()->port();
    BMAG("DispatchJoin: Start on %s:%s\n", localHost.c_str(), localPort.c_str());
    Benchmark* bm = bm_start("ping client->url");
    broker = new MessageBroker(BROKER_FIFO, chain_callback);
    MessageBuffer* buf = new MessageBuffer;
    buf->hostname = host;
    buf->port = port;
    buf->dir = "./public/frontend";
    buf->broker = broker;
    buf->sent = "Echoing before dispatch...";
    buf->fromPort = "8080";
    buf->path = "/ping-local";
    buf->ticket = ++TICKET_ID;
    buf->client = host + ":" + port;

    broker->args((void*)bm);
    broker->epochs(0);
    broker->callbackType("fed-dispatch");
    broker->batchChainUniform(&apiPing, 2, "avg_latency");
    broker->chainReduce("avg_latency");
    broker->chain(&apiDispatch);

    buf->broker = broker;
    router->cluster()->boss()->brokers()[client->url].push_back(broker);
    router->cluster()->boss()->send(router, client->url, std::string("/ping"), buf);
    return "TICKET";
}

API(Train, {}) 
    BYEL("TRAINING...\n");
    std::string content = args["content"];
    std::string path = router->cluster()->boss()->dir();
    std::string dir = "./public/cluster/" + router->cluster()->boss()->port();
    path = "./public/cluster/" + router->cluster()->boss()->port() + "/torch.pt";
    
    Jericho::FileSystem::writeBinary(path.c_str(), content);
    router->federator()->local()->training();
    GUI::state(router, client);

    std::string command_path = "python3 ./py/torch_load.py " + dir;
    std::string results = pipe(command_path);
    std::string wts = dir + "/mnist_train.wt";

    router->federator()->local()->trained();
    GUI::state(router, client);
    GUI::accuracy(router, client, results);

    std::string bytes2 = Jericho::FileSystem::readBinary(wts.c_str());
    std::vector<std::string> connections = router->federator()->connections();
    if (connections.size() > 0) {
        std::string hostPath = connections.at(0) + "/join-weights";
        router->cluster()->boss()->send2(router, client->url, hostPath, "binary", bytes2);
    } else {
        BRED("apiTrain: No connections to send model weights to!\n");
    }

    return "TICKET";
}

API(FedNode, {})
    std::string localHost = router->cluster()->boss()->host();
    std::string localPort = router->cluster()->boss()->port();
    REQUEST_INFO
    BMAG("ENTERING FED NODE...\n");
    std::string content = API_ARG(args, std::string("content"));
    std::string p, result;
    if (content != "undefined") {
        result = "Assigning Fed Node " + content; 
    } else {
        result = "Failed to assign Fed Node: Invalid url given"; 
    }
    BMAG("FedNode Dispatch Request: %s -- %s\n", result.c_str(), content.c_str());
    std::string neighb = content;
    std::string portmanteau;
    if (content.find(":") != std::string::npos) {
        std::vector<std::string> toks = prizm::tokenize(content, ':');
        portmanteau = toks[1];
    }
    if (portmanteau == "" || portmanteau.size() != 4) {
        BRED("CRITICAL -- ApiFedNode: didn't find port\n");
        portmanteau = "8080";
    }
    // add connect logic here
    GUI::disconnectAll(router, client);
    sleep(0.2);
    router->federator()->connect(neighb);
    GUI::connect(router, client, neighb);
    broker = new MessageBroker(BROKER_FIFO, chain_callback);
    broker->epochs(0);
    broker->chain(&apiTrain);
    GUI::state(router, client);

    MessageBuffer* buf = new MessageBuffer;
    buf->hostname = host;
    buf->port = portmanteau;
    buf->dir = "./public/frontend";
    buf->broker = broker;
    buf->sent = "Looking to train";
    buf->fromPort = localPort;
    buf->path = "/serve-fed-model";
    buf->ticket = ++TICKET_ID;
    buf->client = host + ":" + content;
    
    router->cluster()->boss()->send(router, client->url, std::string("/ping"), buf);
    return "TICKET";
}

API(RequestJoin, {})
    std::string localHost = router->cluster()->boss()->host();
    std::string localPort = router->cluster()->boss()->port();
    BMAG("Request Join: Start on %s:%s\n", localHost.c_str(), localPort.c_str());
    std::string resource = API_ARG(args, std::string("resource"));
    broker = new MessageBroker(BROKER_FIFO, single_callback);
    MessageBuffer* buf = router->cluster()->buffer(client->url, "/dispatch-join");
    if (resource != "undefined") { buf->flag = resource; }
    buf->port = "8080";
    buf->broker = broker;
    router->cluster()->boss()->send(router, client->url, "/dispatch-join", buf);
    GUI::state(router, client);
    return "TICKET";
}

API(ServeFedModel, {})
    BMAG("SERVING FED MODEL...\n");
    REQUEST_INFO
    std::string bytes;
    bytes = Jericho::FileSystem::readBinary(router->federator()->local()->model().c_str());
    router->cluster()->boss()->send2(router, client->url, "127.0.0.1:"+port+"/train", "binary", bytes);
    return "TICKET";
}

API(Survey, {})
    BMAG("apiSurvey: SERVING FED MODEL...\n");
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    std::deque<MessageBuffer*> bufs;
    std::vector<std::string> clients = router->federator()->local()->clientUrls();
    for (auto client : clients) {
        MessageBuffer* buf = new MessageBuffer(hostname, client + "/train", router->cluster()->boss()->dir(), content);
        buf->type = "binary";
        if (broker == NULL) {
            BRED("apiSurvey: NULL broker encountered\n");
        } else {
            bufs.push_back(buf);
        }
    }
    if (bufs.size() > 0) {
        for (auto m : bufs) {
            thread_pool_add(router->tpool(), router->worker(), (void *)m);
        }
    }
    else {
        BRED("apiSurvey: No clients\n");
    }
    return "TICKET";
}

API(NewModel, {})
    BYEL("apiNewModel: Saving new Model\n");
    REQUEST_INFO
    std::string content = args["content"];
    std::string path = router->cluster()->boss()->dir();
    BWHI("apiNewModel: Dir: %s\n", path.c_str());
    BWHI("apiNewModel: Content Size: %li\n", content.size());
    std::string localPort = router->cluster()->boss()->port();
    std::string dir = "./public/cluster/" + localPort + "/aggregator/";
    path = dir + port + ".pt";
    router->federator()->local()->model(path);
    BWHI("apiNewModel: New Model Path: %s\n", path.data());
    
    Jericho::FileSystem::writeBinary(path.c_str(), content);
    std::string bytes = Jericho::FileSystem::readBinary(path.c_str());
    
    BRED("apiNewModel: Round: %i, Rounds: %i\n", router->federator()->local()->round(), router->federator()->local()->rounds()); 
    if (router->federator()->local()->finished()) {
        router->federator()->local()->round(0);
        for (auto cli : router->federator()->local()->clientUrls()) {
            std::string name = router->cluster()->boss()->url();
            MessageBuffer* buf = new MessageBuffer(name, cli + "/fed-node", router->cluster()->boss()->dir(), router->cluster()->boss()->port());
            router->cluster()->boss()->send(router, client->url, std::string("/ping"), buf);
        }
    }
    return JsonResponse::success(200, "New model saved");
}

API(JoinWeights, {})
    BYEL("apiJoinWeights: JOINING WEIGHTS...\n");
    REQUEST_INFO
    std::string content = args["content"];
    std::string path = router->cluster()->boss()->dir();
    std::string localPort = router->cluster()->boss()->port();
    std::string dir = "./public/cluster/" + localPort + "/aggregator/";
    path = dir + port + ".wt";

    BWHI("apiJoinWeights: Content Size: %li\n", content.size());
    BWHI("apiJoinWeights: Path: %s\n", path.data());
    
    Jericho::FileSystem::writeBinary(path.c_str(), content);
    std::string bytes = Jericho::FileSystem::readBinary(path.c_str());

    router->federator()->local()->addClient(hostname); // O(n2) for now, not ideal
    std::string mode = "agg";
    if (router->federator()->local()->role() == FED_ROLE_CENTRAL) {
        BRED("WELCOME TO THE CENTRAL NODE\n\n\n");
        mode = "main";
    }

    // if (router->federator()->local()->state() != FL2_TRAINING) {
        if (router->federator()->local()->quorumMet()) {
            BGRE("apiJoinWeights: Quorum met, start aggregation\n");
            std::vector<std::string> selection = router->federator()->local()->clients();
            if (selection.size() != 0) {
                std::string ports = "[";
                for (auto s : selection) {
                    ports += s + ",";
                }
                ports.pop_back(); ports += "]";
                BBLU("apiJoinWeights: Ports: %s\n", ports.c_str());
                std::string name = router->cluster()->boss()->url();
                std::string arg0 = "./py/fusion.py";
                std::string arg1 = ports;
                std::string arg2 = std::to_string(router->federator()->local()->round());
                std::string arg3 = std::to_string(router->federator()->id());
                std::string arg4 = dir;
                std::string arg5 = mode;
                std::string arg6 = router->federator()->local()->model();
                std::string command = "python3 " + arg0 + " " + arg1 + " " + arg2 + " " + arg3 + " " + arg4 + " " + arg5 + " " + arg6;
                router->federator()->local()->training();
                GUI::state(router, client);
                std::string results = pipe(command); // need to account for fusing race condition...?
                GUI::accuracy(router, client, results);
                router->federator()->local()->trained();
                router->federator()->local()->consistencyCheck();
                GUI::state(router, client);
                std::string path;
                if (router->federator()->local()->role() != FED_ROLE_CENTRAL) {
                    path = dir + "agg-" + std::to_string(router->federator()->local()->round()) + ".wt"; 
                } else {
                    path = dir + "model-" + std::to_string(router->federator()->local()->round()) + ".pt"; 
                    router->federator()->local()->model(path);
                }
                std::string bytes = Jericho::FileSystem::readBinary(path.c_str());

                // router->cluster()->index()->quorumUpdate(selection);

                router->federator()->local()->finishRound();
                BRED("apiJoinWeights: Round: %i, Rounds: %i\n", router->federator()->local()->round(), router->federator()->local()->rounds()); 

                if (!router->federator()->local()->finished()) {

                    if (mode != "main") {
                        BBLU("apiJoinWeights: Starting next stage of federation: %i\n", router->federator()->local()->round());
                        broker = new MessageBroker(BROKER_FIFO, single_callback);
                        MessageBuffer* buf = new MessageBuffer(name, hostname + "/fed-node", router->cluster()->boss()->dir(), router->cluster()->boss()->port());
                        buf->broker = broker;
                        router->cluster()->boss()->brokers()[client->url].push_back(broker);
                        router->cluster()->boss()->send(router, client->url, std::string("/ping"), buf);

                    } else {
                        BBLU("apiJoinWeights: Broadcasting new model to children\n");
                        broker = new MessageBroker(BROKER_FIFO, single_callback);
                        MessageBuffer* buf = new MessageBuffer(name, hostname + "/new-model", router->cluster()->boss()->dir(), bytes);
                        buf->type = "binary";
                        buf->broker = broker;
                        router->cluster()->boss()->brokers()[client->url].push_back(broker);
                        router->cluster()->boss()->send(router, client->url, std::string("/ping"), buf);
                    }

                } else {
                    YEL("apiJoinWeights: federator finished\n");
                    std::vector<std::string> outs = router->federator()->local()->outs();
                    // if (router->federator()->reachedTarget()) {
                    // } else {

                    // }
                    router->federator()->local()->finish();
                    GUI::state(router, client);
                    YEL("apiJoinWeights: state sent\n");
                    // GUI::finalLatency(router, client);
                    // YEL("apiJoinWeights: latency sent\n");
                    // GUI::finalRounds(router, client);
                    // YEL("apiJoinWeights: rounds sent\n");
                    // GUI::finalTrain(router, client);
                    // YEL("apiJoinWeights: train sent\n");
                    for (auto out : outs) {
                        MessageBuffer* buf = new MessageBuffer(name, out + "/join-weights", router->cluster()->boss()->dir(), bytes);
                        broker = new MessageBroker(BROKER_FIFO, single_callback);
                        buf->type = "binary";
                        buf->broker = broker;
                        buf->dump();
                        router->cluster()->boss()->brokers()[client->url].push_back(broker);
                        router->cluster()->boss()->send(router, client->url, std::string("/ping"), buf);
                    }
                }
            } else {
                BYEL("apiJoinWeights: No clients available\n");
            }
        } else {
            BRED("Quorum Not Met: Need: %i, Have: %li\n", router->federator()->local()->quorum(), router->federator()->local()->clients().size());
        }
    // } else {
        // router->federator()->queueClient();
        // RED("apiJoinWeights: Aggregator is busy! Should queue!\n");
    // }

    // if (router->cluster()->index()->activateClient(host, port) < 0) {
    //     BRED("Failed to activate client: %s:%s\n", host.c_str(), port.c_str());
    //     BYEL("SIZE OF QUORUM IS: %li\n", router->cluster()->index()->quorum().size());
    // }
    
    return JsonResponse::success(200, "Weights have been joined\n");
    // return "TICKET";
}

API(Aggregate, {})
    REQUEST_INFO
    std::string bytes = API_ARG(args, std::string("content"));
}

API(ResetFederator, {})
    REQUEST_INFO
    router->federator()->local()->reset();
    return "Backend reset federator";
}

API(UpdateFederator, {})
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    BMAG("apiUpdateFederator\n");
    picojson::value data;
    std::string name = "demo-gui";
    std::string cerr = picojson::parse(data, content);
    if (!cerr.empty()) {
        BRED("UpdateFederator: Failed to parse Json!\n");
        return "Failed to update federator";
    }
    router->federator()->local()->update(data.get<picojson::object>());
    return "Updated Federator";
}














// old may reuse or delete

API(Join, {})
    BMAG("JOINING...\n");
    if (client == NULL) {
        BRED("CLIENT IS NULL SOMEHOW :: apiJoin\n");
    }
    // char address_buffer[16];
    // uint16_t p;
	// client_get_full_address(client, address_buffer, &p);
    // std::string port = std::to_string(p);
    // std::string host(address_buffer);
    std::string host;
    std::string port;
    std::string h = API_ARG(args, std::string("Host"));
    if (h != "undefined") {
        std::vector<std::string> vec = tokenize(h, ':');
        host = vec[0];
        port = vec[1];
    }
    // std::string port = API_ARG(args, port);
    // BBLU("Client address is %s, port is %s\n", host.c_str(), port.c_str());
    if (router->needsAggregate()) {
        ClusterQuorum* q = router->cluster()->index()->get(host, port);
        if (q == nullptr) {
            if (router->cluster()->join(host, port)) {
                BGRE("JOINING NEW QUORUM\n");
                for (auto a : router->cluster()->index()->quorum()) {
                    if (a.second == nullptr) {
                        BRED("NULL QUORUM FOUND\n");
                        continue;
                    }
                    BBLU("%s :: %s", a.first.c_str(), a.second->serialize().c_str());
                }
                q = router->cluster()->index()->get(host, port);
            } else {
                BRED("FAILED TO JOIN -- SHOULD NEVER HAPPEN!\n");
                return "{\"status\": 500, \"response\": {\""+port+"\": {\"status\": \"failed\", \"message\": \"Failed to join federation - CRITICAL\"}}}";
            }
        }
        if (q == nullptr) {
            BRED("QUORUM STILL NULL\n");
        }
        q->status = FL_JOINED;
        router->cluster()->index()->quorumTrain(std::vector<ClusterQuorum*>{q});
        BYEL("SIZE OF QUORUM IS: %li\n", router->cluster()->index()->quorum().size());
        for (auto a : router->cluster()->index()->quorum()) {
            if (a.second == nullptr) {
                BRED("NULL QUORUM FOUND\n");
                continue;
            }
            BBLU("%s", a.second->port.c_str());
        }
        router->cluster()->boss()->send2(router, client->url, "127.0.0.1:"+port+"/get-fed-model");
        return "{\"status\": 200, \"response\": {\""+port+"\": {\"status\": \"joined\", \"message\": \"Requesting model\"}}}";
    } else {
        // if (router->cluster()->join(host, port)) {
        //     return "{\"status\": 200, \"response\": {\""+port+"\": {\"status\": \"joined\", \"message\": \"Successfully joined federation\"}}}";
        // }
        // return "{\"status\": 500, \"response\": {\""+port+"\": {\"status\": \"failed\", \"message\": \"Failed to join federation\"}}}";
    }
    return "{\"status\": 500, \"response\": {\""+port+"\": {\"status\": \"failed\", \"message\": \"Failed to join federation\"}}}";
}

API(FederationStatus, {}) 
    BYEL("GETTING FEDERATION STATUS...\n");
    return JsonResponse::error(200, "Federation status placeholder!\n");
}

API(GetFedModel, {})
    BMAG("GETTING FED MODEL...\n");
    std::string h = API_ARG(args, std::string("Host"));
    std::string host;
    std::string port;
    if (h != "undefined") {
        std::vector<std::string> vec = tokenize(h, ':');
        host = vec[0];
        port = vec[1];
    }
    // std::string type = "binary";
    router->cluster()->boss()->send2(router, client->url, "127.0.0.1:"+port+"/serve-fed-model");
    return "TICKET";
}

API(Federation, {})
    BYEL("FEDERATING...\n");

    std::string numClients_ = API_ARG(args, std::string("num-clients"));
    std::string numRounds_ = API_ARG(args, std::string("num-rounds"));
    std::string timeout_ = API_ARG(args, std::string("timeout"));
    if (numClients_ == "undefined") numClients_ = "1";
    if (numRounds_ == "undefined") numRounds_ = "2";
    if (timeout_ == "undefined") timeout_ = "60";
    int timeout = std::stoi(timeout_);
    int numRounds = std::stoi(numRounds_);
    int numClients = std::stoi(numClients_);

    bool has_clients = false;
    if (router->needsAggregate()) {
        BGRE("Clients in cluster detected!\n");
        std::string bytes = Jericho::FileSystem::readBinary("./py/scripts/torch.pt");
        BYEL("HOST BYTES SIZE...? %li\n", bytes.size());
        std::string type = "binary";
        std::vector<ClusterQuorum*> newClients = router->cluster()->index()->selectType(FL_JOINED);
        std::vector<std::pair<std::string, std::string>> set;
        for (auto p : newClients) {
            set.push_back({p->host, p->port});
        }
        if (set.size() > 0) {
            has_clients = true;
            router->cluster()->index()->quorumTrain(newClients);
            router->cluster()->boss()->broadcastNaive(router, client->url, set, "/train", group_callback, type, bytes);
        }
    } else {
        BRED("No clients in cluster detected!\n");
    }

    router->federate(numClients, numRounds, timeout);
    if (has_clients) {
        return "TICKET";
    }
    return JsonResponse::success(200, "No clients in network");
}

API(ResetQuorum, {})
    router->cluster()->index()->resetQuorum();
    return JsonResponse::success(200, "Reset quorum");
}

#endif