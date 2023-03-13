#ifndef JERICHO_API_MESSAGE_BUFFER_H_
#define JERICHO_API_MESSAGE_BUFFER_H_

#include <string>
#include <deque>

#include "server/defs.h"
#include "prizm/prizm.h"

static int TICKET_ID = -1;

class MessageBroker;
class System;
class Client;

struct MessageBuffer {
  
    MessageBuffer() { PCREATE; ticket = ++TICKET_ID; }

    MessageBuffer(std::string src, std::string dest, std::string dir, std::string content) {
        PCREATE;
        std::vector<std::string> toks = prizm::tokenize(src, ':');
        std::vector<std::string> toks2 = prizm::tokenize(dest, ':');
        std::vector<std::string> toks3 = prizm::tokenize(toks2[1], '/');
        this->hostname = toks2[0];
        this->port = toks3[0];
        this->dir = dir;
        this->sent = content;
        this->fromPort = toks[1];
        this->path = "/" + toks3[1];
        this->ticket = ++TICKET_ID;
        this->client = src;
    }

    ~MessageBuffer() { PDESTROY; }
  
    std::string sent = "undefined";
  
    std::string hostname = "undefined";
  
    std::string port = "undefined";

    std::string fromHost = "undefined";

    std::string fromPort = "undefined";
  
    std::string path = "undefined";
  
    std::string received = "undefined";
  
    std::string dir = "undefined";

    std::string type = "undefined";

    std::string flag = "undefined";

    std::string flag2 = "undefined";
  
    int ticket = -1;
  
    MessageBroker* broker = nullptr;

    double latency = 0.0;

    std::string client;
  
    void publish();
  
    void mark();
  
    void dump();
};

typedef std::string (*MessageCallback)(System* router, Client* client, std::deque<MessageBuffer*>, std::string, void*);

#endif