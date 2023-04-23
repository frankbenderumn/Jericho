#include <pthread.h>
#include <semaphore.h>

#include "message/bifrost.h"
#include "system/system.h"
#include "server/request.h"
#include "server/fetch.h"
#include "celerity/celerity.h"
#include "message/message_buffer.h"
#include "message/message_broker.h"
#include "api/api_helper.h"
#include "message/callback.h"
#include "cluster/cluster_node.h"
#include "session/cookie.h"
#include "session/session.h"
#include "server/defs.h"
#include "server/client.h"
#include "util/url.h"

// for client only use, may need to make thread pool more robust to handle behavior that doesn't require incoming connections

void Bifrost::burst(System* router) {
    MAG("Bifrost::serve: Start...\n");
    std::vector<BifrostBurst*> bursts = router->cluster()->boss()->bursts();
    MAG("Bifrost::serve: Burst size: %li\n", bursts.size());
    if (bursts.size() > 0) {
        for (auto burst : bursts) {
            MessageBroker* broker = burst->broker;
            if (broker->hasMessages()) {
                MAG("Bifrost::serve: FIRING ASYNC CALL - EPOCH: %i\n", broker->epoch());
                MAG("Bifrost::serve: Client url is: %s\n", burst->url.c_str());
                MAG("Bifrost::serve: Compiling callback...\n");
                std::deque<MessageBuffer*> bufs = broker->response(burst->url);
                broker->stash(bufs);
                // std::string response = broker->callback()(router, burst->client, bufs, broker->callbackType(), broker->args());
                std::string response = "None";
                MAG("Bifrost::serve: Response: %s\n", response.c_str());    
                if (broker->epoch() == broker->epochs()) {
                    if (isHTTP(response)) {
                        MAG("Bifrost::serve: Serve raw\n");
                        // resource::serve_raw(burst->client, clients, response.c_str());
                    } else {
                        MAG("Bifrost::serve: Serve HTTP\n");
                        // resource::serve_http(burst->client, clients, response.c_str());
                    }
                    // client->promised = false;
                    router->cluster()->boss()->serveBroker(burst->url, broker);
                    MAG("Bifrost::serve: End poll true\n");
                    // drop_client(client, clients); // this segfaults buffer size of 500000 but not 4095 (fixed...?)
                }
            }
        }
    }
}

bool Bifrost::poll(System* router, std::string url, Client* client, Client** clients) {
    MAG("Bifrost::poll: They see me pollin, they hatin\n");
    MessageBroker* broker = router->cluster()->boss()->poll(url);
    if (broker != NULL) {
        MAG("Bifrost::poll: FIRING ASYNC CALL - EPOCH: %i\n", broker->epoch());
        MAG("Bifrost::poll: Client url is: %s\n", url.c_str());
        MAG("Bifrost::poll: Compiling callback...\n");
        std::deque<MessageBuffer*> bufs = broker->response(url);
        broker->stash(bufs);
        // std::string response = broker->callback()(router, client, bufs, broker->callbackType(), broker->args());
        std::string response = "none";
        MAG("Bifrost::poll: Response: %s\n", response.c_str());    
        MAG("Bifrost::poll: Epoch: %i, Epochs: %i\n", broker->epoch(), broker->epochs());    
        if (broker->epoch() - 1 == broker->epochs()) {
            if (isHTTP(response)) {
                MAG("Bifrost::poll: Serve raw\n");
                resource::serve_raw(client, clients, response.c_str());
            } else {
                MAG("Bifrost::poll: Serve HTTP\n");
                resource::serve_http(client, clients, response.c_str());
            }
            // client->promised = false;
            router->cluster()->boss()->serveBroker(url, broker);
            MAG("Bifrost::poll: End poll true\n");
            return true;
            // drop_client(client, clients); // this segfaults buffer size of 500000 but not 4095 (fixed...?)
        }
    }
    MAG("Bifrost::poll: End poll false\n");
    return false;
}

bool authenticate(System* sys, Request* req) {
	bool username = false;
	bool password = false;
	if (req->arg("username") == "joey") { username = true; }
	if (req->arg("password") == "pass1234") { password = true; }
	if (username && password) {
		return true;
	}
	return false;
}

SessionManager* Bifrost::sessionManager() const { return _sm; }

std::string serialize(std::string url, std::string response, std::string command = "null");

std::string batch_serialize(std::vector<std::pair<std::string, std::string>> responses, std::string command = "null");

int Bifrost::fulfill(std::string& response, Request* req, Client* client, Client** clients) {
    int ticket = std::stoi(req->header("Ticket"));
    GRE("Fulfilling ticket: %i\n", ticket);
    Callback* callback = req->callback;
    std::string type = req->header("Broker-Type");

    req->dump();

    if (prizm::contains_key(_brokers, ticket)) {

        std::cout << "Use count: " << _brokers[ticket].use_count() << std::endl;

        // MessageBroker* broker = _brokers[ticket].get();
        BLU("Messages size: %li\n", _brokers[ticket]->messages().size());
        for (auto m : _brokers[ticket]->messages()) {
            m->dump();
        }

        if (callback->resolved) {
            _brokers[ticket].reset();
            _brokers.erase(ticket);
            delete callback;
            return 2;
        }

        if (type == "simple") {
            if (!callback) {
                YEL("Callback empty!\n");
                std::string content = req->content;
                resource::serve_http(client, clients, content.c_str());
                MAG("Content: %s\n", content.c_str());
                std::cout << "Use count: " << _brokers[ticket].use_count() << std::endl;
                _brokers[ticket].reset();
                _brokers.erase(ticket);
                response = content;
                return 1;
            }
        } else if (type == "broadcast") {
            if (!callback) {
                bool all_fulfilled = true;
                std::string content = req->content;
                std::vector<std::pair<std::string, std::string>> responses;
                int message_id = std::stoi(req->header("Message-Id"));
                for (auto m : _brokers[ticket]->messages()) {
                    if (m->fulfilled == 0) {
                        if (m->id == message_id) {
                            m->fulfilled = 1;
                            m->received = content;
                            responses.push_back({m->url, m->received});
                        } else {
                            all_fulfilled = false;
                        }
                    } else {
                        responses.push_back({m->url, m->received});
                    }
                }

                if (all_fulfilled) {
                    response = batch_serialize(responses);
                    BGRE("Broadcast fulfilled:\n");
                    printf("%s\n", response.c_str());
                    _brokers[ticket].reset();
                    _brokers.erase(ticket);
                    return 1;
                }
            }

            
        } else if (type == "ricochet") {
            if(callback) {
                double latency = _brokers[ticket]->messages()[0]->latency;
                BBLU("Latency: %f\n", latency);
                _latencies[ticket].push_back(latency);
                if (callback->count != callback->limit) {
                    if (!callback->satellite) callback->count++;
                    callback->satellite = !callback->satellite;
                    return 2;
                } else {
                    if (callback->fn == "AVG") {
                        double sum = 0;
                        for (auto& l : _latencies[ticket]) {
                            sum += l;
                        }
                        BLU("Avg Latency: %f\n", sum/_latencies[ticket].size());
                        req->content = std::to_string(sum/_latencies[ticket].size());
                    }
                    BGRE("Callback resolved!\n");
                    callback->resolved = 1;
                    return 2;
                }
            }
        } else if (type == "propogate") {
            // the fun stuff. rattlesnake algo, blockchain, ip obfuscation
        }

    } else {
        BRED("Bifrost::fulfill: No ticket found!\n");
    }

    return 0;    
}

void Bifrost::serve(System* sys, Client* client, Client** clients, Request* req) {
    std::string result;
    std::string ipath;
    Route* route;
    Response* resp;
    Cookie* cookie = 0;

    std::string cookie_header = req->header("Cookie");

    if (cookie_header != "undefined") {
        BGRE("Cookie detected!\n");
        std::vector<std::string> toks = prizm::tokenize(cookie_header, "; ");
        if (toks.size() > 1) {
            for (auto t : toks) {
                BYEL("Cookie: %s\n", t.c_str());
            }
        } else {
            BYEL("Cookie: %s\n", cookie_header.c_str());
            std::string::size_type p;
            if ((p = cookie_header.find("-")) != std::string::npos) {
                std::string prefix = cookie_header.substr(0, p);
                std::string cook = cookie_header.substr(p+1, cookie_header.size() - p - 1);
                BWHI("Cook: %s\n", cook.c_str());
                if ((p = cook.find("=")) != std::string::npos) {
                    std::string key = cook.substr(0, p);
                    std::string val = cook.substr(p+1, cook.size() - p - 1);
                    BYEL("Key: %s, Val: %s\n", key.c_str(), val.c_str());
                    std::string addr = client_get_address(client);
                    cookie = new Cookie(addr, key, val);
                }

                if (prefix == "__Host") {
                    BBLU("Host prefix detected!\n");
                } else if (prefix == "__Secure") {
                    BBLU("Secure prefix detected!\n");
                }
            }
        }
    } else {
        BRED("Bifrost::server: No cookie detected!\n");
    }

    printf("Cookie verification...");
    jericho::Session* sesh = 0;
    if (cookie) {
        sesh = this->_sm->verify(cookie);
    }
    
    if (sesh) {
        BGRE(" Session found!\n");
    } else {
        BRED(" Session not found!\n");
    }

    switch (sys->router()->protocol(req->path)) {
        case ROUTE_RAW:
            result = sys->router()->exec(ROUTE_RAW, req->path, req, sys, client);
            resource::serve_http(client, clients, result.c_str());	
            break;									
        case ROUTE_SYSTEM:
            BYEL("System...\n");
            req->dump();
            // BMAG("Sys->router()->dump():\n");
            // sys->router()->dump();
            result = sys->router()->exec(ROUTE_SYSTEM, req->path, req, sys, client);
            if (result == "TICKET") {
                GRE("Bifrost::serve: Ticket Received -- will not drop client for now\n");
                req->async = true;
                resource::serve_http(client, clients, "ticket", std::string("application/json"));
                // exit(1);
            } else if (result == "COMPLETE") {
                // drop_client(client, clients, &sys->num_clients);
            } else {
                GRE("Bifrost::serve: System call sending http result: %.100s\n", result.c_str());
                resource::serve_http(client, clients, result.c_str());		
            }
            break;
        case ROUTE_API:
            BYEL("API CALL...\n");
            result = sys->router()->exec(ROUTE_API, req->path, req, sys, client);
            GRE("Bifrost::serve: API call sending http result: %.100s\n", result.c_str());
            resource::serve_http(client, clients, result.c_str(), std::string("application/json"));
            break;
        case ROUTE_RESOURCE:
            BYEL("RESOURCE CALL...\n");
            resp = sys->router()->resource(sys, req, sesh);
            if (resp) {
                resp->serve();
            } else {
                BRED("Response is NULL\n");
                resource::error(sys->router(), client, "305");
            }
            break;
        case ROUTE_SECURE:
            BYEL("SECURE CALL...\n");
            if (sesh == NULL) {
                BRED("Bifrost::serve: No active session. Redirecting to login...\n");
                req->dump();
                if (authenticate(sys, req)) {
                    route = sys->router()->route(req->path);
                    BGRE("Serving secure route...!\n");
                    if (route->callback) {
                        result = route->callback(req, sys, client, NULL);
                    } else {
                        BRED("No callback detected!\n");
                        PLOG(LSERVER, "\033[1;32mSecure\033[0m route being served.");
                        result = sys->router()->exec(ROUTE_HTTP, req->path, req, sys, client);
                        resource::serve_cxx(sys, client, clients, req->path.c_str());
                    }
                } else {
                    resource::error(sys->router(), client, "305");    
                }
            }

            route = sys->router()->route(req->path);

            if (route) {
                BGRE("Serving secure route...!\n");
                if (route->callback) {
                    result = route->callback(req, sys, client, NULL);
                } else {
                    BRED("No callback detected!\n");
                    PLOG(LSERVER, "\033[1;32mSecure\033[0m route being served.");
                    result = sys->router()->exec(ROUTE_HTTP, req->path, req, sys, client);
                    resource::serve_cxx(sys, client, clients, req->path.c_str());
                }
            } else {
                // add a redirect
                resource::error(sys->router(), client, "305");
            }
            break;
        case ROUTE_IRIS:
            BYEL("IRIS CALL...\n");
            ipath = sys->router()->ipath(req->path);
            resource::serve_cxx(sys, client, clients, ipath.c_str());
            BGRE("Iris call done!\n");
            // drop_client(client, clients, &router->num_clients);
            break;
        case ROUTE_NULL:
        case ROUTE_HTTP:
            result = sys->router()->exec(ROUTE_HTTP, req->path, req, sys, client);
            resource::serve_cxx(sys, client, clients, req->path.c_str());
            break;
        default:
            BRED("UNREGISTERED ROUTE\n");
            resource::serve_http(client, clients, "Route does not exist", std::string("application/json"));
            break;
    }
    if (req->async) {
        BGRE("Async request made!\n");
        // sys->bifrost()->dumpBrokers();
        // exit(1);
    }
}


MessageBuffer* Bifrost::buffer(std::string _url, std::shared_ptr<MessageBroker> broker, std::string dir) {
    URL* url = new URL(_url);
    if (url->ip) {
        MessageBuffer* buf = new MessageBuffer;
        buf->hostname = this->host();
        buf->port = url->port;

        buf->fromHost = this->host();
        buf->fromPort = this->port();

        buf->toHost = url->host;
        buf->toPort = url->port;

        buf->protocol = url->protocol;

        buf->sent = "Ping from " + this->port();
        buf->path = url->path;
        buf->type = url->type;

        if (url->protocol == "jfl") {
            buf->dir = "/cluster";
        } else {
            buf->dir = "undefined";
        }

        if (broker != nullptr) {
            buf->broker = broker;
        }

        buf->id = ++MESSAGE_ID;
        buf->url = url->url;
        delete url;
        return buf;
    }
    delete url;
    return nullptr;
}

std::string Bifrost::broadcast(std::vector<std::string> urls, std::string content, Callback* callback) {
    BBLU("BROADCASTING...\n");
    std::deque<MessageBuffer*> mq;
    std::shared_ptr<MessageBroker> broker = std::make_shared<MessageBroker>(BROKER_BARRIER, callback);
    int ticket = ++TICKET_ID;

    pthread_t threads[urls.size()];
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, urls.size());

    for (auto url : urls) {
        MessageBuffer* buf = this->buffer(url, broker);
        if (content != "") {
            buf->sent = content;
            buf->ticket = ticket;
            buf->modality = 1;
            buf->barrier = &barrier;
            buf->headers["Broker-Type"] = "broadcast";
            BMAG("BROADCAST SIZE: %li\n", content.size());
        }
        buf->dump();
        mq.push_back(buf);
    }
    broker->messages(mq);
    this->broker(ticket, broker);

    for (int i = 0; i < urls.size(); i++) {
        pthread_create(&threads[i], NULL, reinterpret_cast<void* (*)(void*)>(_worker), (void*)mq[i]);
    }

    SEGH

    std::vector<std::pair<std::string, std::string>> responses;
    int status;
    for (int i = 0; i < urls.size(); i++) {
        pthread_join(threads[i], (void**)&status);
        printf("i: %i\n", i);
        if (mq[i] != nullptr) {
            // responses.push_back({"wtf", "hell"});
        } else {
            BRED("%i not found!\n", i);
        }
        printf("i2: %i\n", i);
    }

    for (auto m : mq) {
        responses.push_back({m->url, m->received});
    }

    SEGH

    pthread_barrier_destroy(&barrier);

    SEGH

    return batch_serialize(responses);
}

int Bifrost::broadcast_async(std::vector<std::string> urls, std::string content, Callback* callback) {
    BBLU("BROADCASTING...\n");
    std::deque<MessageBuffer*> mq;
    std::shared_ptr<MessageBroker> broker = std::make_shared<MessageBroker>(BROKER_BARRIER, callback);
    int ticket = ++TICKET_ID;
    for (auto url : urls) {
        MessageBuffer* buf = this->buffer(url, broker);
        if (content != "") {
            buf->sent = content;
            buf->ticket = ticket;
            buf->headers["Broker-Type"] = "broadcast";
            buf->headers["Ticket"] = std::to_string(ticket);
            buf->headers["Message-Id"] = std::to_string(buf->id);
            BMAG("BROADCAST SIZE: %li\n", content.size());
        }
        buf->dump();
        mq.push_back(buf);
    }
    broker->messages(mq);
    this->broker(ticket, broker);

    BBLU("MQ SIZE IS: %li\n", mq.size());
    for (auto m : mq) {
        thread_pool_add(_tpool, _worker, (void*)m);
    }
    return ticket;
}

int Bifrost::send_async(std::string url, std::string content, Callback* callback, int timeout) {
    BBLU("SENDING...\n");
    std::deque<MessageBuffer*> mq;
    int ticket = ++TICKET_ID; 
    std::shared_ptr<MessageBroker> broker = std::make_shared<MessageBroker>(BROKER_BARRIER, callback);
    MessageBuffer* buf = this->buffer(url, broker);
    if (content != "") {
        buf->sent = content;
        buf->size = content.size();
        buf->ticket = ticket;
        buf->headers["Broker-Type"] = "simple";
        buf->headers["Ticket"] = std::to_string(ticket);
        buf->headers["Message-Id"] = std::to_string(buf->id);
        BMAG("SEND SIZE: %li\n", content.size());
        buf->broker = broker;
    }
    buf->dump();
    mq.push_back(buf);
    broker->messages(mq);
    _brokers[ticket] = broker;
    
    BBLU("MQ SIZE IS: %li\n", mq.size());
    for (auto m : mq) {
        thread_pool_add(_tpool, _worker, (void*)m);
    }
    return ticket;
}

std::string Bifrost::send(std::string url, std::string content, Callback* callback, int timeout) {
    BBLU("SENDING...\n");
    std::deque<MessageBuffer*> mq;
    int ticket = ++TICKET_ID; 
    std::shared_ptr<MessageBroker> broker = std::make_shared<MessageBroker>(BROKER_BARRIER, callback);
    MessageBuffer* buf = this->buffer(url, broker);
    if (content != "") {
        buf->sent = content;
        buf->size = content.size();
        buf->ticket = ticket;
        buf->headers["Broker-Type"] = "simple";
        buf->headers["Ticket"] = std::to_string(ticket);
        buf->headers["Message-Id"] = std::to_string(buf->id);
        BMAG("SEND SIZE: %li\n", content.size());
        buf->broker = broker;
    }
    buf->dump();
    mq.push_back(buf);
    broker->messages(mq);
    // _brokers[ticket] = broker;

    pthread_t thread;
    int status;

    pthread_create(&thread, NULL, reinterpret_cast<void* (*)(void*)>(_worker), (void*)mq[0]);
    pthread_join(thread, (void**)&status);

    std::string result = mq[0]->received;
    broker.reset();
    // _brokers[ticket].reset();
    // _brokers.erase(ticket);

    return serialize(url, mq[0]->received);
}

void Bifrost::reply(Request* req, std::string url, std::string content, Callback* callback, std::string brokerType, int timeout) {
    BBLU("REPLYING...\n");
    std::deque<MessageBuffer*> mq;
    int ticket = ++TICKET_ID;
    std::shared_ptr<MessageBroker> broker = std::make_shared<MessageBroker>(BROKER_BARRIER, callback);
    MessageBuffer* buf = this->buffer(url, broker);
    if (content != "") {
        buf->sent = content;
        buf->size = content.size();
        buf->ticket = ticket;
        buf->headers["Ticket"] = req->header("Ticket");
        buf->headers["Broker-Type"] = brokerType;
        buf->headers["Message-Id"] = req->header("Message-Id");
        if (callback) {
            buf->callback(callback);
        }
        BMAG("SEND SIZE: %li\n", content.size());
    }
    buf->dump();
    mq.push_back(buf);
    broker->messages(mq);
    this->broker(ticket, broker);
    
    BBLU("MQ SIZE IS: %li\n", mq.size());
    for (auto m : mq) {
        thread_pool_add(_tpool, _worker, (void*)m);
    }
    // return ticket;
}

void Bifrost::ricochet_reply(Request* req, std::string url, std::string content, Callback* callback, std::string brokerType, int ticket, int timeout) {
    BBLU("REPLYING...\n");
    std::deque<MessageBuffer*> mq;
    // int ticket = ++TICKET_ID;
    std::shared_ptr<MessageBroker> broker = std::make_shared<MessageBroker>(BROKER_BARRIER, callback);
    MessageBuffer* buf = this->buffer(url, broker);
    if (content != "") {
        buf->sent = content;
        buf->size = content.size();
        buf->ticket = ticket;
        buf->headers["Ticket"] = req->header("Ticket");
        buf->headers["Broker-Type"] = brokerType;
        buf->headers["Message-Id"] = req->header("Message-Id");
        if (callback) {
            buf->callback(callback);
        }
        BMAG("SEND SIZE: %li\n", content.size());
    }
    buf->dump();
    mq.push_back(buf);
    broker->messages(mq);
    this->broker(ticket, broker);
    
    BBLU("MQ SIZE IS: %li\n", mq.size());
    for (auto m : mq) {
        thread_pool_add(_tpool, _worker, (void*)m);
    }
    // return ticket;
}

// callback variables:
    // this->url = url;
    // this->endpoint = endpoint;
    // this->type = type;
    // this->internal = internal;
    // this->fn = fn;
    // this->count = count;

// pulse, make a realm?
int Bifrost::ricochet_async(std::string url, std::string endpoint, std::string content, Callback* callback, int timeout) {
    BBLU("RICOCHET ASYNC...\n");
    std::deque<MessageBuffer*> mq;
    std::shared_ptr<MessageBroker> broker = std::make_shared<MessageBroker>(BROKER_BARRIER, callback);
    int ticket = ++TICKET_ID;
    MessageBuffer* buf = this->buffer(url, broker);
    if (content != "") {
        buf->sent = content;
        buf->ticket = ticket;
        buf->headers["Ticket"] = std::to_string(ticket);
        buf->headers["Broker-Type"] = "ricochet";
        if (callback) {
            buf->callback(callback);
        }
        BMAG("BROADCAST SIZE: %li\n", content.size());
    }
    buf->dump();
    mq.push_back(buf);
    broker->messages(mq);
    this->broker(ticket, broker);

    BBLU("MQ SIZE IS: %li\n", mq.size());
    for (auto m : mq) {
        thread_pool_add(_tpool, _worker, (void*)m);
    }

    return ticket;
}


int Bifrost::ricochet(std::string url, std::string endpoint, std::string content, Callback* callback, int timeout) {

    return -1;
}

// int Bifrost::ricochet_reply(Request* request, std::string url, std::string content, Callback* callback, int timeout) {

//     return -1;
// }

std::string serialize(std::string url, std::string response, std::string command) {
    picojson::object o;
    picojson::object subo;
    subo["status"] = picojson::value("200");
    subo["message"] = picojson::value(response);
    subo["url"] = picojson::value(url);
    picojson::array arr; 
    arr.push_back(picojson::value(subo));
    o["responses"] = picojson::value(arr);
    o["command"] = picojson::value(command);
    o["status"] = picojson::value("200");
    picojson::value v(o);
    return v.serialize();
}

std::string batch_serialize(std::vector<std::pair<std::string, std::string>> responses, std::string command) {
    picojson::object o;
    picojson::array main;
    for (auto res : responses) {
        picojson::object subo;
        subo["status"] = picojson::value("200");
        subo["message"] = picojson::value(res.second);
        subo["url"] = picojson::value(res.first);
        main.push_back(picojson::value(subo));
    }
    o["responses"] = picojson::value(main);
    o["command"] = picojson::value(command);
    o["status"] = picojson::value("200");
    picojson::value v(o);
    return v.serialize();
}