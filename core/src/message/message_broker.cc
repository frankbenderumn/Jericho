#include "message/message_broker.h"

MessageBroker::MessageBroker(BrokerType type, MessageCallback callback, int epoch) {
    PCREATE;
    _type = type;
    _callback = callback;
    _epoch = epoch;
}

void MessageBroker::publishMessage(MessageBuffer* mbuf) {
    _tickets[mbuf->ticket] = 0;
    _promised[mbuf->ticket] = mbuf;
}

void MessageBroker::markMessage(MessageBuffer* mbuf) {
    _messages.push_back(mbuf);
    _tickets[mbuf->ticket] = 1;
}

void MessageBroker::broadcast(std::string url, std::deque<MessageBuffer*> mq, std::vector<ClusterNode*> nodes) {
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

bool MessageBroker::ready(std::string url) {
    // BYEL("MESSAGES SIZE: %i\n", (int)_messages.size());
    // BYEL("PROMISED SIZE: %i\n", (int)_promised.size());
    if (_type == BROKER_BARRIER || _type == BROKER_RR) {
        if (_messages.size() == _promised.size() && _messages.size() != 0) return true;
    } else {
        if (_messages.size() > 0) return true;
    }
    return false;
}

void MessageBroker::refresh() {
    _tickets.clear();
    _promised.clear();
    _messages.clear();
}

std::string MessageBroker::reduce() {
    std::string reduction = std::get<2>(_chain[_epoch]);
    BRED("START REDUCTION MF: %s\n", reduction.c_str());
    if (reduction == "avg_latency") {
        _epoch++;
        BGRE("IN REDUCTION MF\n");
        return "0.2";
    }
    _epoch++;
    return "undefined";
}

std::deque<MessageBuffer*> MessageBroker::response(std::string url) {
    BGRE("MessageBroker::response: BROKER RESPONDING\n");
    std::deque<MessageBuffer*> result;
    if (_type == BROKER_BARRIER || _type == BROKER_RR) {
        result = _messages;
        BYEL("MessageBroker::response: Returning messages of size: %i\n", (int)_messages.size());
        for (int i = 0; i < _messages.size(); i++) {
            BYEL("MessageBroker::response: BATCH MSG: ");
            BWHI("%s\n", _messages.at(i)->received.c_str());
        }
        _messages.clear();
        _promised.clear(); // may need to multiple requests sent at once
        _tickets.clear();
    } else {
        if (_messages.size() > 0) {
            MessageBuffer* el = _messages.front();
            BCYA("MessageBroker::response: SINGLE MSG: ");
            BWHI("%s\n", el->received.c_str());
            _messages.pop_front();
            _tickets.erase(el->ticket);
            _promised.erase(el->ticket);
            result.push_back(el);
        } else {
            BRED("MessageBroker::response: BROKER IS EMPTY! This shouldn't happen unless a ticket collision.\n")
        }
    }
    BBLU("MessageBroker::response: %li\n", result.size());
    return result;
}

void MessageBroker::stash(std::deque<MessageBuffer*> buf) {
    BBLU("MessageBroker::stash: stashing...\n");
    for (auto m : buf) {
        _stash.push_back(m);
    }
}

std::deque<MessageBuffer*> MessageBroker::stashed() {
    return _stash;
}