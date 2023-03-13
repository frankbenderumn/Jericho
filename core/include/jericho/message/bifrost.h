#ifndef JERICHO_MESSAGE_BIFROST_H_
#define JERICHO_MESSAGE_BIFROST_H_

#include <unordered_map>
#include <vector>

#include "message/message_broker.h"
#include "util/file_system.hpp"
#include "server/request.h"
#include "session/session.h"

using namespace jericho;

class Bifrost {
    std::unordered_map<std::string, std::vector<MessageBroker*>> _messages;
    SessionManager* _sm;
    // std::unordered_map<std::string, std::vector<MessageResponse*>> _responses;
  public:
    Bifrost() { PCREATE; _sm = new SessionManager; }
    ~Bifrost() { PDESTROY; delete _sm; }

    void burst(System* router);

    bool poll(System* router, std::string url, Client* client, Client** clients);

    void insert(std::string url, MessageBroker* broker) {
        _messages[url].push_back(broker);
    }

    SessionManager* sessionManager() const;

    void dump() {
        std::string result;
        for (auto m : _messages) {
            result += m.first + "==";
            for (auto b : m.second) {
                result += b->stringify() + "--";
            }
        }
        JFS::write("./log/debug.log", result.c_str());
    }

    void serve(System* sys, Client* client, Client** clients, Request* req);

};

#endif