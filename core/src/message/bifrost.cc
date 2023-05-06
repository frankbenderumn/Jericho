#include <pthread.h>
#include <semaphore.h>

#include "message/bifrost.h"
#include "system/system.h"
#include "server/request.h"
#include "server/fetch.h"
#include "celerity/celerity.h"
#include "message/message.h"
#include "message/message_broker.h"
#include "api/api_helper.h"
#include "message/callback.h"
#include "cluster/cluster_node.h"
#include "session/cookie.h"
#include "session/session.h"
#include "server/defs.h"
#include "server/client.h"
#include "util/url.h"
#include "crypt/crypt.h"

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

void Bifrost::cleanse() {
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    BMAG("Bifrost::cleanse: Cleansing...\n");
    // MAG("\tPrecleanse:\n");
    // this->dumpBrokerSizes();
    BCYA("Bifrost::cleanse: Brokers size %li\n", _brokers.size());
    for (auto it = _brokers.begin(); it != _brokers.end(); ) {
        BCYA("Bifrost::cleanse: Broker Message size %li\n", it->second->messages().size());
        CYA("%i : %li\n", it->first, _brokers.size());
        bool toContinue = false;
        BYEL("Bifrost::cleanse: Iterating brokers\n");
        std::deque<Message*> messages;
        if (it->second->messages().size() != 0) {
            messages = it->second->messages();
        } else {
            BRED("Bifrost::cleanse: it is nullptr!\n");
        }
        for (int i = 0; i < messages.size(); i++) {
            Message* buf = messages.at(i);
            YEL("Bifrost::cleanse: Iterating message\n");
            std::chrono::microseconds micro = std::chrono::duration_cast<std::chrono::microseconds>(now - buf->timestamp);
            int diff = static_cast<int>(micro.count());
            MAG("\tCompare: %i == %li\n", diff, buf->timeout);
            if (diff > buf->timeout) {
                BMAG("\tBifrost::cleanse: Async call timed out\n");
                // exit(1);
                // delete messages.at(i);
                messages.erase(messages.begin() + i);
            } else {
                MAG("\tBifrost::cleanse: Call still valid\n");
                // ++it;
            }
            // if (buf->fulfilled == 2) {
            //     BRED("EXPIRED BUFFER FOUND!\n");
            //     exit(1);
            // }
            MAG("Bifrost::cleanse: Finished Iterating Message\n");
        }
        if (messages.size() == 0) {
            // exit(1);
            it->second.reset();
            it = _brokers.erase(it);
        } else {
            ++it;
        }
    }
    auto it = _fnfs.begin();
    while (it != _fnfs.end()) {
        if ((*it)->fulfilled != 0) {
            delete *it;
            it = _fnfs.erase(it);
        } else {
            it++;
        }
    }
    // MAG("\tPostcleanse:\n");
    // this->dumpBrokerSizes();
    // BMAG("Bifrost::cleanse: Done\n");
}

int Bifrost::fulfill(std::string& response, Request* req, Client* client, Client** clients) {
    BMAG("Bifrost::fulfill: Starting...\n");
    int ticket = std::stoi(req->header("Ticket"));
    printf("\tFulfilling ticket: %i\n", ticket);
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

        if (callback) {
            if (callback->resolved) {
                _brokers[ticket].reset();
                _brokers.erase(ticket);
                delete callback;
                return 2;
            }
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
            } else {
                BMAG("Well shit guess we got a callback!\n");
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

    if (!cookie_header.empty()) {
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
        case ROUTE_SYSTEM:
            result = sys->router()->exec(ROUTE_SYSTEM, req->path, req, sys, client);
            if (result == "TICKET") {
                GRE("Bifrost::serve: Ticket Received -- will not drop client for now\n");
            } else {
                GRE("Bifrost::serve: System call sending http result: (%li) %s\n", result.size(), result.c_str());
                std::string proto;
                if (result.size() > 8) {
                    for (int i = 0; i < 8; i++) {
                        proto += result[i];
                    }
                }
                // for (auto& c : result) {
                //     if (c == '\0') {
                //         printf("\033[1;35m0\033[0m");
                //     } else {
                //         printf("%c", c);
                //     }
                // }
                printf("\n");
                if (proto == "HTTP/1.1" || (result[0] == 'G')) {
                    resource::serve_http2(client, clients, result.data(), result.size());
                } else {
                    BRED("Proto: %s\n", proto.c_str());
                    resource::serve_http(client, clients, result.c_str());
                }
            }
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
    }
    BGRE("Bifrost::serv done!\n");
}


Message* Bifrost::buffer(std::string _url, std::shared_ptr<MessageBroker> broker, std::string dir) {
    URL* url = new URL(_url);
    if (url->ip) {
        Message* buf = new Message;
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
    std::deque<Message*> mq;
    std::shared_ptr<MessageBroker> broker = std::make_shared<MessageBroker>(BROKER_BARRIER, callback);
    int ticket = ++TICKET_ID;

    pthread_t threads[urls.size()];
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, urls.size());

    for (auto url : urls) {
        Message* buf = this->buffer(url, broker);
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
    // this->broker(ticket, broker);

    for (int i = 0; i < urls.size(); i++) {
        pthread_create(&threads[i], NULL, reinterpret_cast<void* (*)(void*)>(_worker), (void*)mq[i]);
    }

    std::vector<std::pair<std::string, std::string>> responses;
    int status;
    for (int i = 0; i < urls.size(); i++) {
        pthread_join(threads[i], (void**)&status);
        printf("i: %i\n", i);
        if (mq[i] == nullptr) {
            BRED("%i not found!\n", i);
        }
        printf("i2: %i\n", i);
    }

    BYEL("Message size: %li\n", mq.size());
    for (auto m : mq) {
        responses.push_back({m->url, m->received});
    }

    pthread_barrier_destroy(&barrier);

    broker.reset();

    return batch_serialize(responses);
}

int Bifrost::broadcast_async(std::vector<std::string> urls, std::string content, Callback* callback) {
    BBLU("BROADCASTING...\n");
    std::deque<Message*> mq;
    std::shared_ptr<MessageBroker> broker = std::make_shared<MessageBroker>(BROKER_BARRIER, callback);
    int ticket = ++TICKET_ID;
    for (auto url : urls) {
        Message* buf = this->buffer(url, broker);
        if (content != "") {
            buf->sent = content;
            buf->ticket = ticket;
            buf->headers["Broker-Type"] = "broadcast";
            buf->headers["Ticket"] = std::to_string(ticket);
            buf->headers["Message-Id"] = std::to_string(buf->id);
            BMAG("BROADCAST SIZE: %li\n", content.size());
        }
        // buf->dump();
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
    BMAG("Bifrost::send_async: Starting...\n");
    std::deque<Message*> mq;
    int ticket = ++TICKET_ID; 
    std::shared_ptr<MessageBroker> broker = std::make_shared<MessageBroker>(BROKER_BARRIER, callback);
    Message* buf = this->buffer(url, broker);
    if (content != "") {
        buf->sent = content;
        buf->size = content.size();
        buf->ticket = ticket;
        buf->headers["Broker-Type"] = "simple";
        buf->headers["Ticket"] = std::to_string(ticket);
        buf->headers["Message-Id"] = std::to_string(buf->id);
        buf->broker = broker;
        if (callback) {
            buf->callback(callback);
        }
    }

    mq.push_back(buf);
    broker->messages(mq);
    _brokers[ticket] = broker;

    for (auto m : mq) {
        thread_pool_add(_tpool, _worker, (void*)m);
    }
    return ticket;
}

std::string Bifrost::send(std::string url, std::string content, Callback* callback, std::string response_format, int timeout) {
    BBLU("SENDING...\n");
    std::deque<Message*> mq;
    int ticket = ++TICKET_ID; 
    std::shared_ptr<MessageBroker> broker = std::make_shared<MessageBroker>(BROKER_BARRIER, callback);
    Message* buf = this->buffer(url, broker);
    if (content != "") {
        buf->sent = content;
        buf->size = content.size();
        // buf->ticket = ticket; // only use if messagebuffer needs to reference a ticket, should make more intuitive
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

    std::string type = buf->type;

    pthread_create(&thread, NULL, reinterpret_cast<void* (*)(void*)>(_worker), (void*)mq[0]);
    pthread_join(thread, (void**)&status);

    std::string result = mq[0]->received;

    // BCYA("Bifrost::send (Client %s):\n", this->hostname().c_str());
    // printf("\t\033[1;37m%s\033[0m: %s\n", "Received", mq[0]->received.c_str());

    broker.reset();
    // _brokers[ticket].reset();
    // _brokers.erase(ticket);

    if (response_format == "json" || type == "json") {
        return serialize(url, result);
    }

    return result;
}

std::string Bifrost::send(Message* msg) {
    pthread_t thread;
    int status;

    pthread_create(&thread, NULL, reinterpret_cast<void* (*)(void*)>(_worker), (void*)msg);
    pthread_join(thread, (void**)&status);

    std::string result = msg->received;
    // delete msg;
    return result;
}


void Bifrost::reply(Request* req, std::string url, std::string content, Callback* callback, std::string brokerType, int timeout) {
    BBLU("REPLYING...\n");
    // std::deque<Message*> mq;
    // MessageBroker* broker = new MessageBroker(BROKER_BARRIER, callback);
    Message* buf = new Message(TICKET_ID, url, this->host(), this->port());
    if (content != "") {
        MAG("Content: %s\n", content.c_str());
        buf->sent = content;
        buf->size = content.size();
        buf->ticket = std::stoi(req->header("Ticket"));
        buf->headers["Ticket"] = req->header("Ticket");
        buf->headers["Broker-Type"] = brokerType;
        buf->headers["Message-Id"] = req->header("Message-Id");
        if (callback) {
            buf->callback(callback);
        }
    }

    // mq.push_back(buf);
    // broker->messages(mq);
    _fnfs.push_back(buf);
    BMAG("FNFS SIZE: %li\n", _fnfs.size());
    
    // for (auto m : mq) {
    thread_pool_add(_tpool, _worker, (void*)buf);

    // delete broker;
    
    BBLU("Reply brokers @\n");
    dumpBrokerSizes();
    // broker.reset();
    // return ticket;
}

void Bifrost::ricochet_reply(Request* req, std::string url, std::string content, Callback* callback, std::string brokerType, int ticket, int timeout) {
    BBLU("RICOCHET REPLYING...\n");
    std::deque<Message*> mq;
    // int ticket = ++TICKET_ID;
    std::shared_ptr<MessageBroker> broker = std::make_shared<MessageBroker>(BROKER_BARRIER, callback);
    Message* buf = this->buffer(url, broker);
    if (content != "") {
        buf->sent = content;
        buf->size = content.size();
        buf->ticket = std::stoi(req->header("Ticket"));
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
    std::deque<Message*> mq;
    std::shared_ptr<MessageBroker> broker = std::make_shared<MessageBroker>(BROKER_BARRIER, callback);
    int ticket = ++TICKET_ID;
    Message* buf = this->buffer(url, broker);
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

std::string Bifrost::get_file(std::string remote_host, std::string file_path, int fd, Callback* callback, int timeout) {
    std::string job_token = jcrypt::base64::encode_url(jcrypt::generate_session_token(32));
    size_t chunk_size = 2048;
    std::string file, moniker;
    size_t p;
    if ((p = file_path.find(" AS ")) != std::string::npos) {
        file = file_path.substr(0, p);
        moniker = file_path.substr(p+4, file_path.size() - p - 4);
    } else {
        file = file_path;
        moniker = file_path;
    }
    BMAG("Bifrost::get_file: Getting FTP...\n"); 
    std::string url = "https://" + remote_host + "/read-file";
    Message* hs = new Message(TICKET_ID, url, this->host(), this->port());
    hs->headers["Job-Id"] = job_token;
    hs->headers["Connection"] = "keep-alive";
    hs->headers["Host"] = this->host() + ":" + this->port();
    hs->sent = file;
    hs->simple = true;
    std::string handshake = this->send(hs);
    delete hs;
    Message* handy = new Message(handshake);
    MAG("\tBifrost::get_file: Received handshake: %s\n", handshake.c_str());
    if (handy->directive == HttpStatus::response(100)) {
        std::string all;
        std::string sz_str = handy->header("Content-Length");
        size_t content_sz;
        if (!parseSize(content_sz, sz_str)) {
            delete handy;
            this->clear_job_id(job_token);
            return HttpStatus::response(500);
        }
        std::string empty;
        std::ofstream ofs(moniker, std::ios::out | std::ios::trunc);
        ofs.close();
        int what = content_sz;
        int num_chunks = ceil(static_cast<double>(content_sz) / chunk_size);
        for (int i = 0; i < num_chunks; i++) {
            BYEL("Content_sz: %i\n", what);
            BYEL("num_chunks: %i\n", num_chunks);
            BYEL("chunk_idx: %i\n", i);
            Message* buf = new Message(TICKET_ID, url, this->host(), this->port());
            buf->method = "GET";
            buf->headers["Broker-Type"] = "simple";
            buf->headers["Message-Id"] = std::to_string(buf->id);
            buf->headers["Job-Id"] = job_token;
            size_t start, end;
            if (i == num_chunks - 1) {
                start = i * chunk_size;
                end = what;
            } else {
                start = i * chunk_size;
                end = (i+1) * chunk_size;
            }
            BYEL("start: %li\n", start);
            BYEL("end: %li\n", end);
            buf->headers["Range"] = "bytes="+std::to_string(start)+"-"+std::to_string(end);
            buf->sent = file;
            pthread_t thread;
            int status;
            pthread_create(&thread, NULL, reinterpret_cast<void* (*)(void*)>(_worker), (void*)buf);
            pthread_join(thread, (void**)&status);
            std::string result = buf->received;
            delete buf;
            printf("\t\033[1;33mBifrost::send_file\033[0m: Received:\n%s\n", result.c_str());
            Message* shit = new Message(result);
            if (shit->directive != HttpStatus::response(206)) {
                this->clear_job_id(job_token);
                delete handy;
                delete shit;
                return HttpStatus::response(500);
            } else {
                all += shit->sent;
                BYEL("Response->sent size: %li\n", shit->sent.size());
                if (!JFS::writeOffset(moniker.c_str(), start, shit->sent)) {
                    BRED("Bifrost::get_file: Failed to writeOffset file '%s'\n", moniker.c_str());
                    this->clear_job_id(job_token);
                    delete handy;
                    delete shit;
                    return HttpStatus::response(500);
                }
            }
            delete shit;
        }
        BGRE("Bifrost::get_file: Sizeof all is %li\n", all.size());
        size_t check_sz;
        if (!JFS::size(check_sz, file.c_str())) {
            BRED("Failed to get size of file '%s'\n", file.c_str());
            delete handy;
            return HttpStatus::response(500);
        }
        size_t written_sz;
        if (!JFS::size(written_sz, moniker.c_str())) {
            BRED("Failed to get size of file moniker '%s'\n", moniker.c_str());
            this->clear_job_id(job_token);
            delete handy;
            return HttpStatus::response(500);
        }
        if (written_sz == check_sz) {
            BGRE("Files match!\n");
            this->clear_job_id(job_token);
            delete handy;
            return HttpStatus::response(200);
        } else {
            BRED("Files do not match %li != %li\n", written_sz, check_sz);
            delete handy;
            this->clear_job_id(job_token);
            return HttpStatus::response(500);
        }
    }
    this->clear_job_id(job_token);
    delete handy;
    return HttpStatus::response(500);
}

// consider using a Expect: header
std::string Bifrost::send_file(std::string remote_host, std::string file_path, Callback* callback, int timeout) {
    BMAG("Bifrost::send_file: Posting FTP...\n");    
    std::string job_token = jcrypt::base64::encode_url(jcrypt::generate_session_token(32));
    size_t buf_size = 2048;
    size_t chunk_size = buf_size;
    std::string url = "https://" + remote_host + "/write-file";
    size_t p = file_path.find(" AS ");
    std::string to_send, to_write;
    if (p != std::string::npos) {
        to_send = file_path.substr(0, p);
        to_write = file_path.substr(p+4, file_path.size() - p - 4);
    } else {
        to_send = file_path;
        to_write = file_path;
    }
    size_t file_sz;
    if (!JFS::size(file_sz, to_send.c_str())) {
        BRED("Failed to get file size for send file '%s'\n", to_send.c_str());
    }
    std::string checksum = JFS::sha256_checksum(to_send);
    size_t num_chunks = ceil(static_cast<double>(file_sz) / buf_size);
    std::ifstream file(to_send, std::ios::binary);
    if (!file.is_open()) {
        BRED("\tBifrost::send_file: Error opening file '%s'!\n", to_send.c_str());
        return "HTTP/1.1 500 Internal Server Error";
    }
    if (num_chunks > 1) {
        BGRE("-------------------\nCHUNKS GREATE THAN 1\n--------------------\n");
        std::string job_id = "applesauce";
        size_t all_size;
        Message* hs = new Message(TICKET_ID, url, this->host(), this->port());
        hs->headers["Job-Id"] = job_token;
        hs->headers["Connection"] = "keep-alive";
        hs->headers["Host"] = this->host() + ":" + this->port();
        hs->simple = true;
        hs->sent = to_write;
        std::string handshake = this->send(hs);
        delete hs;
        MAG("\tBifrost::send_file: Received handshake: %s\n", handshake.c_str());
        if (handshake != "HTTP/1.1 100 Continue") {
            return HttpStatus::response(502);
        }

        for (int i = 0; i < num_chunks; i++) {
            BGRE("Get some?\n");
            size_t start, end;
            if (i == num_chunks - 1) {
                start = chunk_size * i;
                end = file_sz;
            } else {
                start = chunk_size * i;
                end = chunk_size * (i+1);
            }
            BYEL("\tstart: %li\n", start);
            BYEL("\tend: %li\n", end);
            std::string chunkStr = JFS::readOffset(to_send.c_str(), start, end);
            BYEL("\tCHUNK STR SIZE: %li\n", chunkStr.size());
            int ticket = ++TICKET_ID;
            Message* buf = new Message(TICKET_ID, url, this->host(), this->port());
            buf->method = "POST";
            buf->headers["Host"] = this->host() + ":" + this->port();
            buf->headers["Content-Disposition"] = "attachment; filename=" + to_write;
            buf->headers["Content-Type"] = "application/octet-stream";
            buf->headers["Content-Range"] = "bytes " + std::to_string(start) + "-" + std::to_string(end) + "/" + std::to_string(file_sz);
            buf->sent = chunkStr;
            buf->headers["Content-Length"] = std::to_string(chunkStr.size());
            buf->headers["Job-Id"] = job_token;
            buf->simple = true;

            pthread_t thread;
            int status;
            pthread_create(&thread, NULL, reinterpret_cast<void* (*)(void*)>(_worker), (void*)buf);
            pthread_join(thread, (void**)&status);
            std::string result = buf->received;
            delete buf;
            all_size += result.size();
            // BGRE("====================\nCHECK THIS SHIT\n========================\n");
            // BGRE("\tBifrost::send_file: Received: %s\n", result.c_str());
        }

        // verification checksum
        Message* buf2 = new Message(TICKET_ID, "https://"+remote_host+"/verify-write", this->host(), this->port());
        buf2->method = "GET";
        buf2->type = "text/plain";
        buf2->headers["Content-Disposition"] = "attachment; filename=" + to_write;
        buf2->headers["Content-Type"] = "application/octet-stream";
        buf2->headers["Digest"] = "SHA-256=" + JFS::sha256_checksum(to_write);
        buf2->headers["Job-Id"] = job_token;

        pthread_t thread2;
        int status2;
        pthread_create(&thread2, NULL, reinterpret_cast<void* (*)(void*)>(_worker), (void*)buf2);
        pthread_join(thread2, (void**)&status2);
        std::string result = buf2->received;
        delete buf2;

        MAG("\tBifrost::send_file: Received: %s\n", result.c_str());

        return result;
    }
    return "HTTP/1.1 500 Internal Server Error";
}

int Bifrost::parse_handshake(const std::string& heartbeat) {
    BMAG("Bifrost::parse_handshake: Parsing: %s\n", heartbeat.c_str());
    return 0;
}

int Bifrost::handshake(std::string url, std::string endpoint, std::string content, Callback* callback, int timeout) {
    return -1;
}

// int Bifrost::ricochet_reply(Request* request, std::string url, std::string content, Callback* callback, int timeout) {

//     return -1;
// }

std::string serialize(std::string url, std::string response, std::string command) {
    // BRED("Bifrost::serialize DEBUG:\n");
    // printf("\tResponse: %s\n", response.c_str());
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