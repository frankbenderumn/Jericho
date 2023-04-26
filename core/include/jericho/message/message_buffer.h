#ifndef JERICHO_API_MESSAGE_BUFFER_H_
#define JERICHO_API_MESSAGE_BUFFER_H_

#include <string>
#include <deque>
#include <memory>

#include "server/defs.h"
#include "prizm/prizm.h"
#include "message/callback2.h"
#include "util/url.h"

static int MBUF_ID = -1;

class MessageBroker;
class System;
class Client;

struct MessageBuffer {
  
    MessageBuffer() { PCREATE; }
    MessageBuffer(int& ticket, std::string _url);

    MessageBuffer(std::string src, std::string dest, std::string dir, std::string content) {
        PCREATE;
        BRED("Creating message buffer!\n");
        RED("%s\n", src.c_str());
        RED("%s\n", dest.c_str());
        RED("%s\n", dir.c_str());
        std::vector<std::string> toks;
        std::vector<std::string> toks2;
        std::vector<std::string> toks3;
        if (src.find(":") != std::string::npos) { toks = prizm::tokenize(src, ':'); }
        if (dest.find(":") != std::string::npos) { toks2 = prizm::tokenize(src, ':'); }
        if (toks2.size() >= 2) {
            if (src.find(":") != std::string::npos) { toks3 = prizm::tokenize(toks2[1], ':'); }
        }
        // toks2 = prizm::tokenize(dest, ':');
        // toks3 = prizm::tokenize(toks2[1], '/');
        BGRE("Successfully tokenized!\n");
        (toks2.size() >= 1) ? this->hostname = toks2[0] : this->hostname = "undefined";
        (toks3.size() >= 1) ? this->port = toks3[0] : this->port = "undefined";
        this->dir = dir;
        this->sent = content;
        (toks.size() >= 2) ? this->fromPort = toks[1] : this->port = "undefined";
        (toks3.size() >= 2) ? this->path = "/" + toks3[1] : this->path = "undefined";
        this->client = src;
        this->id = ++MBUF_ID;
    }

    ~MessageBuffer() { BMAG("Deleting MessageBuffer\n"); PDESTROY; }
  
    int ticket = -1;
    int id = -1;
    int modality = 0;
    int fulfilled = 0;
    double latency = 0.0;
    size_t size = 0;
    long timeout = 5000000;

    std::string hostname = "undefined";
    std::string port = "undefined";

    std::string fromHost = "undefined";
    std::string fromPort = "undefined";
    std::string sent = "undefined";

    std::string toHost;
    std::string toPort;
    std::string received = "undefined";

    std::string client;
    std::string url;
    std::unordered_map<std::string, std::string> headers;

    std::string protocol;
    std::string path = "undefined";
    std::string dir = "undefined";
    std::string type = "text/plain";

    std::string flag = "undefined";
    std::string flag2 = "undefined";

    std::chrono::high_resolution_clock::time_point timestamp = std::chrono::high_resolution_clock::now();

    pthread_barrier_t* barrier = nullptr;
    std::weak_ptr<MessageBroker> broker;
  
    void publish();
  
    void mark();
  
    void dump();

    void callback(Callback* callback) {
        callback->serialize(this->headers);
    }
};

typedef std::string (*MessageCallback)(System* router, Client* client, std::deque<MessageBuffer*>, std::string, void*);

#endif