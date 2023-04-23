#ifndef JERICHO_MESSAGE_BIFROST_H_
#define JERICHO_MESSAGE_BIFROST_H_

#include <unordered_map>
#include <vector>

#include "util/file_system.hpp"

#include "server/request.h"
#include "session/session.h"

#include "message/message_job.h"
#include "message/message_broker.h"
#include "message/callback2.h"

using namespace jericho;

static int TICKET_ID = -1;

typedef void (*WorkerThread)(void*);

class Bifrost {
    std::unordered_map<std::string, std::vector<std::shared_ptr<MessageBroker>>> _messages;
    SessionManager* _sm;
    ThreadPool* _tpool;
    WorkerThread _worker;
    std::unordered_map<int, std::shared_ptr<MessageBroker>> _brokers;
    std::unordered_map<int, std::vector<double>> _latencies;
    std::unordered_map<int, std::vector<long>> _bandIn;
    std::unordered_map<int, std::vector<long>> _bandOut;
    std::string _host;
    std::string _port;
    std::deque<MessageJob*> _jobs;

    // std::unordered_map<std::string, std::vector<MessageResponse*>> _responses;
  public:
    Bifrost(std::string host, std::string port, ThreadPool* tpool, WorkerThread worker) {
        PCREATE;
        _sm = new SessionManager;
        _host = host;
        _port = port;
        _tpool = tpool;
        _worker = worker;
    }
    ~Bifrost() { PDESTROY; delete _sm; }

    const std::string host() const { return _host; }
    const std::string port() const { return _port; }
    const std::string hostname() const { return _host + ":" + _port; }

    void burst(System* router);

    bool poll(System* router, std::string url, Client* client, Client** clients);

    void insert(std::string url, std::shared_ptr<MessageBroker> broker) {
        _messages[url].push_back(broker);
    }

    void broker(int ticket, std::shared_ptr<MessageBroker> broker) {
        _brokers[ticket] = broker;
    }

    SessionManager* sessionManager() const;

    void serve(System* sys, Client* client, Client** clients, Request* req);

    int broadcast_async(std::vector<std::string> urls, std::string content, Callback* callback);

    std::string broadcast(std::vector<std::string> urls, std::string content, Callback* callback);

    int send_async(std::string url, std::string content, Callback* callback, int timeout = 5);

    std::string send(std::string url, std::string content, Callback* callback, int timeout = 5);

    void reply(Request* request, std::string url, std::string content, Callback* callback, std::string brokerType = "simple", int timeout = 5);

    int ricochet_async(std::string url, std::string endpoint, std::string content, Callback* callback, int timeout = 5);

    int ricochet(std::string url, std::string endpoint, std::string content, Callback* callback, int timeout = 5);

    void ricochet_reply(Request* req, std::string url, std::string content, Callback* callback, std::string brokerType, int ticket, int timeout = 5);

    int fulfill(std::string& response, Request* req, Client* cli, Client** clients);

    MessageBuffer* buffer(std::string _url, std::shared_ptr<MessageBroker> broker = nullptr, std::string dir = "undefined");

    /** DEBUG **/

    void dumpBrokers() {
        for (auto brokers : _brokers) {
            BYEL("Broker Ticket: %i\n", brokers.first);
            for (auto broker : brokers.second->messages()) {
                broker->dump();
            }
        }
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

    void dumpBrokerSizes() {
        for (auto brokers : _brokers) {
            BYEL("Broker Ticket: %i -> %li\n", brokers.first, brokers.second->messages().size());
        }
    }

};

#endif