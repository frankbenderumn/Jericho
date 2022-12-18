#ifndef JERICHO_MESSAGE_BIFROST_H_
#define JERICHO_MESSAGE_BIFROST_H_

#include <unordered_map>
#include <vector>

#include "message/message_broker.h"
#include "util/file_system.hpp"

class Bifrost {
    std::unordered_map<std::string, std::vector<MessageBroker*>> _messages;
    // std::unordered_map<std::string, std::vector<MessageResponse*>> _responses;
  public:
    Bifrost() {}
    ~Bifrost() {}

    void burst(Router* router);

    bool poll(Router* router, std::string url, Client* client, Client** clients);

    void insert(std::string url, MessageBroker* broker) {
        _messages[url].push_back(broker);
    }

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

};

#endif