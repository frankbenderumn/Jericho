#ifndef JERICHO_SERVER_REQUEST_H_
#define JERICHO_SERVER_REQUEST_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <chrono>

#include "util/string_utils.h"

#include "prizm/prizm.h"
#include "server/defs.h"
#include "server/resource.h"
#include "server/client.h"
#include "message/callback2.h"

typedef std::chrono::high_resolution_clock hrclock;

enum ContentType {
    CONTENT_OCTET,
    CONTENT_JSON,
    CONTENT_PLAIN
};

enum EncodingType {
    ENCODING_NONE,
    ENCODING_BASE64
};

struct Request {
    std::string content;
    int size;
    ContentType type = CONTENT_PLAIN;
    EncodingType encoding = ENCODING_NONE;
    std::unordered_map<std::string, std::string> headers;
    std::string method;
    std::string path;
    std::string request;
    std::unordered_map<std::string, std::string> args;
    std::string protocol;
    std::string signature;
    std::string host;
    Callback* callback = nullptr;
    bool async = false;
#ifdef TEST_MODE
    std::string headersStr;
#endif
    int buffer_limit;
    Client* client;
    bool valid = true;
    Request(Client* _client, int _buffer_limit, bool test = false) { 
        PCREATE;
        buffer_limit = _buffer_limit;
        client = _client;
        if (!test) {
            poll_request(_client, _buffer_limit);
        } else {
            poll_request_test(_client, _buffer_limit);
        }
    }
    Request() { PCREATE; }
    ~Request() { 
        PDESTROY; 
        if (callback) {
            delete callback;
        } 
    }

    // helps with binary files
    void poll_request(Client* _client, int _buffer_limit) {
        bool all_read = false;
        // time_t timeout = std::time(NULL);
        auto t1 = hrclock::now();
        auto t2 = hrclock::now();
        if (_client == NULL) {
            BRED("Request::poll_request: Client is null!\n");
        }
        while (!all_read) {
            int r = SSL_read(client->ssl, client->request + client->received, _buffer_limit - client->received);
            if (r > 0) {    
                char* q = strstr(client->request, "\r\n\r\n");
                if (q) {
                    all_read = true;
                    this->request = std::string(client->request);
                    all_read = true;
                } else {
                    BRED("Request::poll_request: Http headers not found!\n");
                    RED("%s\n", client->request);
                }
                client->received += r;
                // BMAG("Request::poll_request: Number of bytes received from client is %i\n", client->received);
                PLOG(LSERVER, "Number of bytes received \033[0;34m%i\033[0m", client->received);
            } else {
                all_read = true;
            }
        }

        // timeout attempt (needs work)
        // t2 = hrclock::now();
        // bold(); green();
        // std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1);
        // printf("duration: %lli\n", ns);
        // clearcolor();
    }

    void validate() {
        if (method == "") { BRED("Request::validate: Method is undefined!\n"); this->valid = false; }
        if (path == "") { BRED("Request::validate: Path is undefined!\n"); this->valid = false; }
        if (protocol == "") { BRED("Request::validate: Protocol is undefined!\n"); this->valid = false; }
    }

    void dump() {
        BYEL("===============\n");
        BYEL("   REQUEST\n");
        BYEL("---------------\n");
        BYEL("protocol: %s\n", this->protocol.c_str());
        BYEL("method: %s\n", this->method.c_str());
        BYEL("path: %s\n", this->path.c_str());
        BYEL("content: %s\n", this->content.c_str());
        BYEL("content-size: %li\n", this->content.size());
        for (auto head : this->headers) {
            BYEL("header: %-32s %s\n", head.first.c_str(), head.second.c_str());
        }
        // BYEL("request: \r\n"); 
        // BCYA("%s\n", request->request.c_str());
        for (auto arg : this->args) {
            BYEL("arg: %-32s %s\n", arg.first.c_str(), arg.second.c_str());
        }
        BYEL("===============\n");
    }

    void poll_request_test(Client* _client, int _buffer_limit) {
        bool all_read = false;
        // time_t timeout = std::time(NULL);
        auto t1 = hrclock::now();
        auto t2 = hrclock::now();
        if (_client == NULL) {
            BRED("Request::poll_request: Client is null!\n");
        }
        while (!all_read) {
            if (strlen(client->request) != 0) {    
                char* q = strstr(client->request, "\r\n\r\n");
                if (q) {
                    all_read = true;
                    // this->request = std::string(q + 4);
                    CYA("Request::poll_request: Content %s\n", q);
                    CYA("Request::poll_request: Content-Size %li\n", this->request.size());
                    all_read = true;
                } else {
                    BRED("Request::poll_request: Http headers not found!\n");
                    RED("%s\n", client->request);
                }
                client->received += strlen(client->request) + 1;
                BMAG("Request::poll_request: Number of bytes received from client is %i\n", client->received);
                BMAG("Request::poll_request: Request is %i\n", client->received);
            } else {
                all_read = true;
            }
        }
    }

    std::string parseProtocol() {
        // BYEL("Parsing protocol...\n");
        // BYEL("%s\n", client->request);
        std::string msg(client->request);
        std::string::size_type p = msg.find("\r\n\r\n");
        if (p != std::string::npos) {
            std::string partition = msg.substr(0, p);
            // BMAG("Partition is: %s\n", partition.c_str());
            this->content = msg.substr(p+4, msg.size());
#ifdef TEST_MODE
            this->headersStr = partition;
#endif
        } else {
            BRED("Request::parseProtocol: No \\r\\n\\r\\n found!\n");
            BRED("Report: %s\n", client->request);
        }
        return msg;
    }

    void eval();

    void parseHeaders(std::string headerStr);

    std::string header(std::string key) {
        if (prizm::contains_key(this->headers, key)) {
            return this->headers[key];
        } else {
            BRED("Request::header: Header %s does not exist!\n", key.c_str());
        }
        return "";
    }

    std::string arg(std::string key) {
        if (prizm::contains_key(this->args, key)) {
            return this->args[key];
        } else {
            BRED("Request::arg: Arg %s does not exist!\n", key.c_str());
        }
        return "";
    }

    std::string reply(std::string protocol, std::string path) {
        return protocol + "://" + host + path;
    }
};

bool is_valid_request(Client* client);

int parse_request(Client* client, Request* request);

void print_request(Request* request);

bool isHTTP(std::string request);

#endif