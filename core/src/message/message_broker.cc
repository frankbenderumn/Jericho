#include "message/message_broker.h"

MessageBroker::MessageBroker(BrokerType type, MessageCallback callback, int epoch) {
    _type = type;
    _callback = callback;
    _epoch = epoch;
}

void MessageBroker::publishMessage(MessageBuffer* mbuf) {
    _tickets[mbuf->ticket] = 0;
    _promised[mbuf->ticket] = mbuf;
}

int MessageBroker::epoch() { return _epoch; }

void MessageBroker::epoch(int epoch) { _epoch = epoch; }

void MessageBroker::markMessage(MessageBuffer* mbuf) {
    _messages.push_back(mbuf);
    _tickets[mbuf->ticket] = 1;
    // BBLU("MESSSSSSAGGEGEGEG MARKEDDDDDDDD\n");
}

void MessageBroker::broadcast(Client* client, std::deque<MessageBuffer*> mq, std::vector<ClusterNode*> nodes) {
    if (mq.size() != nodes.size()) {
        BRED("Message Queue and Nodes to broadcast aren't same size!");
        return;
    }
    for (int i = 0; i < mq.size(); i++) {
        _tickets[mq[i]->ticket] = 0;
        _promised[mq[i]->ticket] = mq[i];
    }
    BMAG("PROMISED SIZE IS: %i\n", (int)_promised.size());
}

MessageCallback MessageBroker::callback() const { return _callback; }

bool MessageBroker::hasMessages() {
    return (_messages.size() > 0);
}

bool MessageBroker::hasTickets() {
    return (_tickets.size() > 0);
}

// std::unordered_map<Client*, std::deque<MessageBuffer*>> messages() const {
//     return _messages;
// }

// std::unordered_map<Client*, MessageBuffer*> promised() const {
//     return _promised;
// }

bool MessageBroker::ready(Client* client) {
    // BYEL("MESSAGES SIZE: %i\n", (int)_messages.size());
    if (_type == BROKER_BARRIER || _type == BROKER_RR) {
        if (_messages.size() == _promised.size() && _messages.size() != 0) return true;
    } else {
        if (_messages.size() > 0) return true;
    }
    return false;
}

// bool promised(Client* client) {
//     if (_promised.find(client) != _promised.end()) {
//         return true;
//     }
//     return false;
// }

void MessageBroker::refresh() {
    _tickets.clear();
    _promised.clear();
    _messages.clear();
}

std::deque<MessageBuffer*> MessageBroker::response(Client* client) {
    BGRE("BROKER RESPONDING\n");
    std::deque<MessageBuffer*> result;
    if (_type == BROKER_BARRIER || _type == BROKER_RR) {
        result = _messages;
        BYEL("Returning messages of size: %i\n", (int)_messages.size());
        for (int i = 0; i < _messages.size(); i++) {
            BYEL("BATCH MSG: ");
            BWHI("%s\n", _messages.at(i)->received.c_str());
        }
        _messages.clear();
        _promised.clear(); // may need to multiple requests sent at once
        _tickets.clear();
    } else {
        if (_messages.size() > 0) {
            MessageBuffer* el = _messages.front();
            BCYA("SINGLE MSG: ");
            BWHI("%s\n", el->received.c_str());
            _messages.pop_front();
            _tickets.erase(el->ticket);
            _promised.erase(el->ticket);
            // if (_messages[client].size() == 0) {
            //     _messages.erase(client);
            // }
            result.push_back(el);
        } else {
            BRED("BROKER IS EMPTY! This shouldn't happen unless a ticket collision.\n")
        }
    }
    return result;
}

void MessageBroker::stash(std::deque<MessageBuffer*> buf) {
    BBLU("STASHING\n-\n-\n-\n");
    for (auto m : buf) {
        _stash.push_back(m);
    }
}

std::deque<MessageBuffer*> MessageBroker::stashed() {
    return _stash;
}