#include <ctime>

#include "cluster/cluster_node.h"
#include "system/system.h"
#include "message/message_broker.h"
#include <sys/stat.h>
#include "system/gui.h"

// std::string single_callback(System* router, Client* client, std::deque<Message*> mq, std::string type, void* args) {
//     std::string url = client->url;
//     std::string result = "undefined";
//     if (mq.size() > 0) {
//         BMAG("single_callback: ACCUMULATING SINGLE CALLBACK\n");
//         result = mq.at(0)->received;
//         // delete mq.at(0)->broker;
//         long size = mq.at(0)->received.size();
//         double latency = mq.at(0)->latency;
//         int epoch = mq[0]->broker->epoch();
//         long out = mq.at(0)->sent.size();
//         mq[0]->broker->epoch(++epoch);
//         GUI::latency(router, client, std::to_string(latency)); 
//         GUI::bandwidth(router, client, std::to_string(size), "in");
//         GUI::bandwidth(router, client, std::to_string(out), "out");
//         router->federator()->local()->accumLatency(latency);
//         return mq.at(0)->received;
        
//     } else {
//         BRED("single_callback: SIZE OF SINGLE MQ SHOULD NOT BE 0\n");
//     }
//     return result;
// }

// std::string group_callback(System* router, Client* client, std::deque<Message*> mq, std::string type, void* args) {
//     std::string url = client->url;
//     std::string batch;
//     long size = -1;
//     double latency = -1;
//     long out = -1;
//     int i = 0;
//     for (auto m : mq) {
//         std::string received = m->received;
//         std::string format = "\""+m->port+"\": " + received + ",";
//         batch += format;
//         long size2 = mq.at(i)->received.size();
//         long out2 = mq.at(i)->sent.size();
//         double latency2 = mq.at(i)->latency;
//         if (size2 > size) size = size2;
//         if (latency2 > latency) latency = latency2;
//         if (out2 > out) out = out2;
//         i++;
//         if (received.find("false") == std::string::npos) {
//             BMAG("BOCA RATON\n");
//             BMAG("%s\n", m->flag.c_str());
//             // GUI::tolerantState(router, client, "127.0.0.1:" + m->port, "online");
//         } else {
//             BMAG("%s\n", m->flag.c_str());
//             // GUI::tolerantState(router, client, "127.0.0.1:" + m->port, "offline");
//         }

//     }
//     GUI::latency(router, client, std::to_string(latency)); 
//     GUI::bandwidth(router, client, std::to_string(size), "in");
//     GUI::bandwidth(router, client, std::to_string(out), "out");
//     router->federator()->local()->accumLatency(latency);
//     batch.pop_back();
//     std::string result = "{\"status\": 200, \"response\": {" + batch + "}}"; 

//     if (mq.size() > 0) {
//         BMAG("ACCUMULATING GROUP CALLBACK\n");
//         int epoch = mq[0]->broker->epoch();
//         mq[0]->broker->epoch(++epoch);
//         // delete mq.at(0)->broker;
//     } else {
//         BRED("SIZE OF GROUP MQ SHOULD NOT BE 0\n");
//     }

//     return result;
// }

// std::string epoch_callback(System* router, Client* client, std::deque<Message*> mq, std::string type, void* args) {
//     std::string url = client->url;
//     std::string result;
//     MessageBroker* broker;
//     for (auto m : mq) {
//         result += m->received;
//     }

//     if (mq.size() > 0) {
//         // BMAG("DELETING BROKER\n");
//         BMAG("ACCUMULATING EPOCH CALLBACK\n");
//         // delete mq.at(0)->broker;
//         std::string path = mq[0]->path;
//         broker = mq[0]->broker;
//         int epoch = mq[0]->broker->epoch();
//         mq[0]->broker->epoch(++epoch);
//         mq[0]->broker->stash(mq);
//         BYEL("THIS IS THE BIG ONE\n\n\n\n\n");
//         std::deque<Message*> bufs = broker->stashed();
//         for (auto b : bufs) {
//             BYEL("%s==",b->received.c_str());
//         }
//         BYEL("\nEpoch info result: %s\n", result.c_str());
//         BYEL("Epoch is %i\n==============\n",epoch);
//         if (epoch != 0) {
//             mq[0]->broker->refresh();
//             router->cluster()->boss()->pulse(router, url, path, broker);
//         }
//     } else {
//         BRED("SIZE OF GROUP MQ SHOULD NOT BE 0\n");
//     }

//     if (broker != nullptr) {
//         std::deque<Message*> bufs = broker->stashed();
//         std::string result;
//         for (auto b : bufs) {
//             result += b->received + "-";
//         }
//         return result;
//     }
//     return "epoch callback - FEDERATION FAILED";
// }

// std::string bm_callback(System* router, Client* client, std::deque<Message*> mq, std::string type, void* args) {
//     std::string url = client->url;
//     std::string result = "undefined";
//     if (mq.size() > 0) {
//         BMAG("BMCallback: ACCUMULATING BM CALLBACK\n");
//         result = mq.at(0)->received;
//         // delete mq.at(0)->broker;
//         int epoch = mq[0]->broker->epoch();
//         mq[0]->broker->epoch(++epoch);
//         result = mq.at(0)->received;
//     } else {
//         BRED("BMCallback: SIZE OF SINGLE MQ SHOULD NOT BE 0\n");
//     }

//     if (type == "fed-dispatch") {
//         // Benchmark* bm = (Benchmark*)args;
//         double d = 1.0;
//         // double t = (double)(bm->end.tv_nsec - bm->start.tv_nsec);
//         // printf("Raw timespec.time_t: %f\n", t / 1000000000);
//         // std::string s = std::to_string(t / 1000000000);
//         // write_file("log/timestamp.log", s.c_str());
//         // std::string c = "dispatching " + mq.at(0)->hostname + "...";
//         // write_file("log/timestamp.log", c.c_str());

//         MessageBroker* brok = mq[0]->broker;
//         int e = brok->epoch();
//         BBLU("BMCallback: CURRENT EPOCH: %i\n", e);
//         // brok->epoch(--e);
//         if (brok->epoch() == 1) {
//             std::string agg = router->federator()->dispatch(mq.at(0)->hostname, d);
//             result = agg;
//             brok->refresh();
//             Message* buf = new Message;
//             BWHI("BMCallback: CLIENT URL IS: %s\n", url.c_str());
//             if (url.find(":") != std::string::npos) {
//                 std::vector<std::string> toks = prizm::tokenize(url, ':');
//                 buf->hostname = toks[0];
//                 buf->port = toks[1];
//                 buf->dir = "./public/cluster/" + toks[0];
//             }
//             buf->broker = brok;
//             buf->sent = agg;
//             buf->fromPort = "8080";
//             buf->path = "/fed-node";
//             buf->ticket = ++TICKET_ID;
//             buf->client = url;
//             BBLU("BMCallback: Phase 1 result: %s\n", result.c_str());
//             router->cluster()->boss()->send(router, url, "/fed-node", buf);
//         } else {
//             BBLU("BMCallback: Phase 2 result: %s\n", result.c_str());
//         }
    
//     } else {
//         BWHI("BMCallback: CALLBACK TYPE: %s\n", type.c_str());
//     }

//     BGRE("BMCallback: END BM CALLBACK\n");
//     BBLU("BMCallback: Result: %s\n", result.c_str());

//     return result;
// }

// std::string chain_callback(System* router, Client* client, std::deque<Message*> mq, std::string type, void* args) {
//     std::string url = client->url;
//     std::string result = "undefined";
//     if (mq.size() > 0) {
//         BMAG("ChainCallback: ACCUMULATING BM CALLBACK\n");
//         result = mq.at(0)->received;
//         long size = mq.at(0)->received.size();
//         double latency = mq.at(0)->latency;
//         long out = mq.at(0)->sent.size();
//         GUI::latency(router, client, std::to_string(latency)); 
//         GUI::bandwidth(router, client, std::to_string(size), "in");
//         GUI::bandwidth(router, client, std::to_string(out), "out");
//         router->federator()->local()->accumLatency(latency);
//         // delete mq.at(0)->broker;
//         int epoch = mq[0]->broker->epoch();
//         mq[0]->broker->epoch(++epoch);
//         result = mq.at(0)->received;
//     } else {
//         BRED("ChainCallback: SIZE OF SINGLE MQ SHOULD NOT BE 0\n");
//     }

//     double d = 1.0;

//     MessageBroker* brok = mq[0]->broker;
//     int e = brok->epoch();
//     Request* req = new Request;
//     BBLU("ChainCallback: CURRENT EPOCH: %i\n", e - 1);
//     if (brok->epoch() - 1 != brok->epochs()) {
//         BBLU("ChainCallback: What is this bullshit\n");
//         Args arglist;
//         BBLU("ChainCallback: Declared arglist\n");
//         arglist["content"] = result;
//         req->args["content"] = result;
//         BBLU("ChainCallback: Set content\n");
//         arglist["Host"] = client->url;
//         req->args["Host"] = client->url;
//         BBLU("ChainCallback: Set url\n");
//         if (brok->isRPC()) {
//             BBLU("ChainCallback: Chaining\n");
//             brok->chain()(req, router, client, brok);
//         } else {
//             BBLU("ChainCallback: Reducing\n");
//             std::string reduction = brok->reduce();
//             // std::string reduction = "DUMB SHIT";
//             BBLU("ChainCallback: Reduction is: %s\n", reduction.c_str());
//             arglist["content"] = reduction;
//             BBLU("ChainCallback: Reduction assigned\n");
//             brok->chain()(req, router, client, brok);
//             BBLU("ChainCallback: Chain continued\n");
//         }
//     } else {
//         BBLU("ChainCallback: Phase End result: %s\n", result.c_str());
//     }

//     BGRE("ChainCallback: END BM CALLBACK\n");
//     BBLU("ChainCallback: Result: %s\n", result.c_str());
//     for (auto m : mq) {
//         delete m;    
//     }
//     return result;
// }

ClusterNode::ClusterNode(std::string host, std::string port, std::string dir, ClusterIndex* index) {
    _host = host;
    _port = port;
    _dir = dir;
    _index = index;
    _id = ++CLUSTER_ID;
    _timestamp = std::time(0);
    DIR* dr;
    struct dirent* en;
    dr = opendir(dir.c_str()); //open all or present directory

    if ( !dr ) {
        BRED("INVALID DIRECTORY PROVIDED FOR CLUSTER NODE: %s\n", dir.c_str());
        const int dir_err = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (-1 == dir_err) {
            printf("Error creating directory %s!\n", dir.c_str());
        } else {
            BGRE("Created directory: %s\n", dir.c_str());
            dr = opendir(dir.c_str());
            if ( !dr ) {
                BRED("Cluster node constructor. Likely a permissions issue!\n");
            }
        }
    }

    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            printf("%s\n", en->d_name); //print all directory name
            _files.push_back(std::string(en->d_name));
            _index->addFile(std::string(en->d_name), _id);
            _index->addNode(this);
        }
        closedir(dr); //close all directory
    }
    BGRE("Added cluster node\n");
}

ClusterNode::~ClusterNode() {
    BRED("DELETING CLUSTER NODE\n");
    if (_edge != NULL) {
        delete _edge;
    }
}

void ClusterNode::brokerBroadcast(System* sys, std::string url, std::deque<Message*> mq, MessageCallback callback) {
    // MessageBroker* broker = new MessageBroker(BROKER_BARRIER, callback);
    for (int i = 0; i < mq.size(); i++) {
        // mq.at(i)->broker = broker;
        // mq.at(i)->ticket = ++TICKET_ID;
        // if (url == "/ping-local") {
        //     mq.at(i)->flag = "gui-status";
        // }
    }
    // broker->messages(mq);
    // broker->broadcast(url, mq, _edge->nodes());
    // if (url == NULL) {
    //     _fetchBrokers[1].push_back(broker);
    // } else {
        // _brokers[url].push_back(broker);
        // sys->bifrost()->broker(++TICKET_ID, broker);
    // }

    BBLU("MQ SIZE IS: %li\n", mq.size());
    for (auto m : mq) {
        thread_pool_add(sys->tpool(), sys->worker(), (void*)m);
    }
}

void ClusterNode::pulse(System* router, std::string url, std::string path, MessageBroker* broker) {
    std::deque<Message*> mq;
    for (auto n : _edge->nodes()) {
        Message* buf = n->buffer(url, path);
        buf->dump();
        mq.push_back(buf);
    }
    // for (int i = 0; i < mq.size(); i++) {
    //     mq.at(i)->broker = broker;
    // }
    // broker->broadcast(url, mq, _edge->nodes());
    if (!containsKey(_brokers, url)) {
        // _brokers[url].push_back(broker);
    }
    for (auto m : mq) {
        thread_pool_add(router->tpool(), router->worker(), (void*)m);
    }
}

std::vector<BifrostBurst*> ClusterNode::bursts() { return _bursts; }

void ClusterNode::addBurst(BifrostBurst* burst) { _bursts.push_back(burst); }


MessageBroker* ClusterNode::poll(std::string url) {
    // BBLU("Brokers size: %li\n", _brokers.size());
    if (url != "undefined") {
        if (containsKey(_brokers, url)) {
            // BYEL("PENDING BROKER...\n");
            // std::vector<int> timed = {};
            for (int i = 0; i < _brokers[url].size(); i++) {
                MessageBroker* broker = _brokers[url].at(i);
                // if (!broker->timeout()) {
                    if (broker->ready(url)) {
                        return broker;
                    }
                // } else {
                //     timed.push_back(i);
                // }
            }
        }
    } 
    return nullptr;
}

void ClusterNode::serveBroker(std::string client, MessageBroker* broker) {
    std::string idx = client;
    int toErase = -1;
    for (auto url : _brokers) {
        int i = 0;
        BYEL("ClusterNode::serverBroker: Client brokers: %li\n", url.second.size());
        for (auto b : url.second) {
            if (broker == b) {
                PLOG(LSERVER, "Broker match found");
                if (broker->completed()) {
                    toErase = i;
                    break;
                }
            }
            i++;
        }
    }
    
    if (toErase != -1) {
        _brokers[idx].erase(_brokers[idx].begin() + toErase);
    }
}

bool ClusterNode::hasEdge(std::string host, std::string port) {
    if (_edge == NULL) return false;
    for (auto n : _edge->nodes()) {
        if (n->port() == port && n->host() == host) {
            return true;
        }
    }
    return false;
}

void ClusterNode::broadcastNaive(System* router, std::string url, std::vector<std::pair<std::string, std::string>> pairs, std::string path, MessageCallback callback, std::string type, std::string content) {
    BBLU("NAIVE BROADCAST...\n");
    std::deque<Message*> mq;
    for (auto hp : pairs) {
        Message* buf = new Message;
        buf->hostname = hp.first;
        buf->port = hp.second;
        buf->sent = "Ping from " + hp.first + ":" + hp.second;
        if (content != "") {
            buf->sent = content;
        }
        buf->fromPort = _port;
        buf->path = path;
        if (type != "") {
            buf->type = type;
        }
        buf->dir = "./public/cluster/" + hp.first;
        // buf->ticket = ++TICKET_ID;
        buf->client = url;
        // buf->dump();
        mq.push_back(buf);
    }
    brokerBroadcast(router, url, mq, callback);
}

void ClusterNode::broadcast(System* router, std::string url, std::string path, MessageCallback callback, std::string type, std::string content) {
    BBLU("BROADCASTING...\n");
    std::deque<Message*> mq;
    for (auto n : _edge->nodes()) {
        Message* buf = n->buffer(url, path);
        if (content != "") {
            buf->sent = content;
            BMAG("BROADCAST SIZE: %li\n", content.size());
        }
        if (type != "") {
            buf->type = type;
        } else {
            buf->type = "http";
        }
        buf->dump();
        mq.push_back(buf);
    }
    brokerBroadcast(router, url, mq, callback);
}

void ClusterNode::pingOne(System* router, std::string url, ClusterNode* dest) {
    Message* buf = dest->buffer(url, "/ping-local");
    MessageBroker* broker = new MessageBroker(BROKER_FIFO, NULL);
    buf->dump();
    // buf->broker = broker;
    
    _brokers[url].push_back(broker);
    send(router, url, "/ping-local", buf);
}

void ClusterNode::pingAll(System* router, std::string url, std::vector<std::pair<std::string, std::string>> set) {
    if (set.size() == 0) {
        broadcast(router, url, "/ping-local", NULL);
    } else {
        std::string response = "Ping from " + _port;
        broadcastNaive(router, url, set, "/ping-local", NULL, "http", response);
    }
}

void ClusterNode::federate(System* router, std::string url, std::string path, int epochs, int clients) {
    MessageBroker* broker = new MessageBroker(BROKER_RR, NULL, epochs);
    pulse(router, url, path, broker);
}

void ClusterNode::brokerSend(System* router, std::string url, std::string path, Message* buf, std::string type, std::string content) {
    MessageBroker* broker = new MessageBroker(BROKER_FIFO, NULL);
    // buf->broker = broker;
    
    _brokers[url].push_back(broker);
    send(router, url, path, buf);
}

void ClusterNode::send2(System* router, std::string url, std::string path, std::string type, std::string content) {
    std::string route = "undefined";
    std::string port = "undefined";
    std::string host = "undefined";
    std::string name = "undefined";
    std::string::size_type p;
    if ((p = path.find("/")) != std::string::npos) {
        name = path.substr(0, p);
        route = path.substr(p, path.size());
    } else {
        BRED("PATH DOES NOT HAVE A /\n");
    }
    if ((p = name.find(":")) != std::string::npos) {
        host = name.substr(0, p);
        port = name.substr(p+1, name.size());
    } else {
        BRED("PATH DOES NOT HAVE A :\n");
    }
    Message* buf = this->buffer(url, path);
    buf->type = type;
    buf->sent = content;
    buf->path = route;
    buf->port = port;
    buf->hostname = host;
    MessageBroker* broker = new MessageBroker(BROKER_FIFO, NULL);
    // buf->broker = broker;
    _brokers[url].push_back(broker);
    send(router, url, path, buf);
}

const int ClusterNode::id() const { return _id; }

void ClusterNode::send(System* router, std::string url, std::string path, Message* buf) {
    thread_pool_add(router->tpool(), router->worker(), (void*)buf);
}

Message* ClusterNode::buffer(std::string url, std::string path) {
    Message* buf = new Message;
    buf->hostname = _host;
    buf->port = _port;
    buf->fromPort = _port;
    buf->sent = "Ping from " + _port;
    buf->path = path;
    buf->dir = _dir;
    buf->ticket = ++TICKET_ID;
    // client->promised = true;
    buf->client = url;
    return buf;
}

Message* ClusterNode::buffer2(std::string url, std::string path, std::string content) {
    Message* buf = new Message;
    buf->hostname = _host;
    buf->port = _port;
    buf->fromPort = _port;
    buf->sent = content;
    buf->path = path;
    buf->dir = _dir;
    buf->ticket = ++TICKET_ID;
    // client->promised = true;
    buf->client = url;
    return buf;
}

ClusterEdge* ClusterNode::edges() const {
    return _edge;
}

const std::vector<ClusterNode*>& ClusterNode::nodes() {
    if (_edge == nullptr) {
        BYEL("EDGE IS NULL\n");
        _edge = new ClusterEdge;
    }
    return _edge->nodes();
}

void ClusterNode::addNode(ClusterNode* node) { 
    if (_edge == nullptr) {
        _edge = new ClusterEdge(CLUSTER_EDGE_STRICT, node);
        BBLU("Creating edge\n");
    } else {
        BYEL("Saruman is an istari!\n");
        _edge->addNode(node);
        BBLU("Adding node to edge\n");
    }
}

ClusterNode* ClusterNode::getEdge(std::string host, std::string port) { 
    if (_edge == NULL) return nullptr;
    for (auto n : _edge->nodes()) {
        BRED("Host: %s, Port: %s\n", n->host().c_str(), n->port().c_str());
        if (n->port() == port && n->host() == host) {
            return n;
        }
    }
    return nullptr;
}

void ClusterNode::addEdge(ClusterEdge* edge) { _edge->addEdge(edge); }

ClusterNodeType ClusterNode::type() { return _type; }

void ClusterNode::type(ClusterNodeType type) { _type = type; }

const std::string ClusterNode::host() const { return _host; }

const std::string ClusterNode::port() const { return _port; }

const long ClusterNode::timestamp() const { return _timestamp; }

void ClusterNode::print() {
    BCYA("%-16lld %-16s %-16s %-16s\n", _timestamp, _host.c_str(), _port.c_str(), _dir.c_str());
}