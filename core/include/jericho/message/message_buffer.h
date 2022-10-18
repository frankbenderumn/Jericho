#ifndef JERICHO_API_MESSAGE_BUFFER_H_
#define JERICHO_API_MESSAGE_BUFFER_H_

#include <string>
#include <deque>

#include "server/defs.h"

class MessageBroker;
class Router;
class Client;

struct MessageBuffer {
  
    MessageBuffer() {}
  
    std::string sent = "undefined";
  
    std::string hostname = "undefined";
  
    std::string port = "undefined";
  
    std::string path = "undefined";
  
    std::string received = "undefined";
  
    std::string dir = "undefined";
  
    int ticket = -1;
  
    MessageBroker* broker = nullptr;
  
    Client* client;
  
    void publish();
  
    void mark();
  
    void dump();
};

typedef std::string (*MessageCallback)(Router* router, Client* client, std::deque<MessageBuffer*>);

#endif