#include <ctime>

#include "cluster/cluster_node.h"
#include "router/router.h"
#include "message/message_broker.h"
#include <sys/stat.h>

std::string single_callback(Router* node, Client* client, std::deque<MessageBuffer*> mq) {
    std::string result = "undefined";
    if (mq.size() > 0) {
        BMAG("DELETING BROKER\n");
        result = mq.at(0)->received;
        // delete mq.at(0)->broker;
        int epoch = mq[0]->broker->epoch();
        mq[0]->broker->epoch(--epoch);
        return mq.at(0)->received;
    } else {
        BRED("SIZE OF SINGLE MQ SHOULD NOT BE 0\n");
    }
    return result;
}

std::string group_callback(Router* node, Client* client, std::deque<MessageBuffer*> mq) {
    std::string batch;
    for (auto m : mq) {
        std::string received = m->received;
        std::string format = "\""+m->port+"\": " + received + ",";
        batch += format;
    }
    batch.pop_back();
    std::string result = "{\"status\": 200, \"response\": {" + batch + "}}"; 

    if (mq.size() > 0) {
        BMAG("DELETING BROKER\n");
        int epoch = mq[0]->broker->epoch();
        mq[0]->broker->epoch(--epoch);
        // delete mq.at(0)->broker;
    } else {
        BRED("SIZE OF GROUP MQ SHOULD NOT BE 0\n");
    }

    return result;
}

std::string epoch_callback(Router* router, Client* client, std::deque<MessageBuffer*> mq) {
    std::string result;
    MessageBroker* broker;
    for (auto m : mq) {
        result += m->received;
    }

    if (mq.size() > 0) {
        // BMAG("DELETING BROKER\n");
        // delete mq.at(0)->broker;
        std::string path = mq[0]->path;
        broker = mq[0]->broker;
        int epoch = mq[0]->broker->epoch();
        mq[0]->broker->epoch(--epoch);
        mq[0]->broker->stash(mq);
        BYEL("THIS IS THE BIG ONE\n\n\n\n\n");
        std::deque<MessageBuffer*> bufs = broker->stashed();
        for (auto b : bufs) {
            BYEL("%s==",b->received.c_str());
        }
        BYEL("\nEpoch info result: %s\n", result.c_str());
        BYEL("Epoch is %i\n==============\n",epoch);
        if (epoch != 0) {
            mq[0]->broker->refresh();
            router->cluster()->boss()->pulse(router, client, path, broker);
        }
    } else {
        BRED("SIZE OF GROUP MQ SHOULD NOT BE 0\n");
    }

    if (broker != nullptr) {
        std::deque<MessageBuffer*> bufs = broker->stashed();
        std::string result;
        for (auto b : bufs) {
            result += b->received + "-";
        }
        return result;
    }
    return "epoch callback - FEDERATION FAILED";
}

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

void ClusterNode::brokerBroadcast(Router* router, Client* client, std::deque<MessageBuffer*> mq, MessageCallback callback) {
    MessageBroker* broker = new MessageBroker(BROKER_BARRIER, callback);
    for (int i = 0; i < mq.size(); i++) {
        mq.at(i)->broker = broker;
    }
    // broker->broadcast(client, mq, _edge->nodes());
    _brokers[client].push_back(broker);
    for (auto m : mq) {
        thread_pool_add(router->tpool(), router->worker(), (void*)m);
    }
}

void ClusterNode::pulse(Router* router, Client* client, std::string path, MessageBroker* broker) {
    std::deque<MessageBuffer*> mq;
    for (auto n : _edge->nodes()) {
        MessageBuffer* buf = n->buffer(client, path);
        buf->dump();
        mq.push_back(buf);
    }
    for (int i = 0; i < mq.size(); i++) {
        mq.at(i)->broker = broker;
    }
    // broker->broadcast(client, mq, _edge->nodes());
    if (!containsKey(_brokers, client)) {
        _brokers[client].push_back(broker);
    }
    for (auto m : mq) {
        thread_pool_add(router->tpool(), router->worker(), (void*)m);
    }
}


MessageBroker* ClusterNode::poll(Client* client) {
    if (containsKey(_brokers, client)) {
        // BYEL("PENDING BROKER...\n");
        for (int i = 0; i < _brokers[client].size(); i++) {
            MessageBroker* broker = _brokers[client].at(i);
            if (broker->ready(client)) {
                return broker;
            }
        }
    }
    return nullptr;
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

void ClusterNode::broadcastNaive(Router* router, Client* client, std::vector<std::pair<std::string, std::string>> pairs, std::string path, MessageCallback callback, std::string type, std::string content) {
    BBLU("NAIVE BROADCAST...\n");
    std::deque<MessageBuffer*> mq;
    for (auto hp : pairs) {
        MessageBuffer* buf = new MessageBuffer;
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
        buf->ticket = ++TICKET_ID;
        client->promised = true;
        buf->client = client;
        buf->dump();
        mq.push_back(buf);
    }
    brokerBroadcast(router, client, mq, callback);
}

void ClusterNode::broadcast(Router* router, Client* client, std::string path, MessageCallback callback, std::string type, std::string content) {
    BBLU("BROADCASTING...\n");
    std::deque<MessageBuffer*> mq;
    for (auto n : _edge->nodes()) {
        MessageBuffer* buf = n->buffer(client, path);
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
    brokerBroadcast(router, client, mq, callback);
}

void ClusterNode::pingOne(Router* router, Client* client, ClusterNode* dest) {
    MessageBuffer* buf = dest->buffer(client, "/ping-local");
    MessageBroker* broker = new MessageBroker(BROKER_FIFO, single_callback);
    buf->dump();
    buf->broker = broker;
    
    _brokers[client].push_back(broker);
    send(router, client, "/ping-local", buf);
}

void ClusterNode::pingAll(Router* router, Client* client, std::vector<std::pair<std::string, std::string>> set) {
    if (set.size() == 0) {
        broadcast(router, client, "/ping-local", group_callback);
    } else {
        std::string response = "Ping from " + _port;
        broadcastNaive(router, client, set, "/ping-local", group_callback, "http", response);
    }
}

void ClusterNode::federate(Router* router, Client* client, std::string path, int epochs, int clients) {
    MessageBroker* broker = new MessageBroker(BROKER_RR, epoch_callback, epochs);
    pulse(router, client, path, broker);
}

void ClusterNode::brokerSend(Router* router, Client* client, std::string path, MessageBuffer* buf, std::string type, std::string content) {
    MessageBroker* broker = new MessageBroker(BROKER_FIFO, single_callback);
    buf->broker = broker;
    
    _brokers[client].push_back(broker);
    send(router, client, path, buf);
}

void ClusterNode::send2(Router* router, Client* client, std::string path, std::string type, std::string content) {
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
    MessageBuffer* buf = this->buffer(client, path);
    buf->type = type;
    buf->sent = content;
    buf->path = route;
    buf->port = port;
    buf->hostname = host;
    MessageBroker* broker = new MessageBroker(BROKER_FIFO, single_callback);
    buf->broker = broker;
    _brokers[client].push_back(broker);
    send(router, client, path, buf);
}

const int ClusterNode::id() const { return _id; }

void ClusterNode::send(Router* router, Client* client, std::string path, MessageBuffer* buf) {
    thread_pool_add(router->tpool(), router->worker(), (void*)buf);
}

MessageBuffer* ClusterNode::buffer(Client* client, std::string path) {
    MessageBuffer* buf = new MessageBuffer;
    buf->hostname = _host;
    buf->port = _port;
    buf->fromPort = _port;
    buf->sent = "Ping from " + _port;
    buf->path = path;
    buf->dir = _dir;
    buf->ticket = ++TICKET_ID;
    client->promised = true;
    buf->client = client;
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