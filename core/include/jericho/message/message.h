#ifndef JERICHO_API_MESSAGE_BUFFER_H_
#define JERICHO_API_MESSAGE_BUFFER_H_

#include <string>
#include <deque>
#include <memory>

#include "server/defs.h"
#include "prizm/prizm.h"
#include "message/callback2.h"
#include "util/url.h"
#include "util/file_system.hpp"
#include "crypt/base64.h"
#include "server/http_status.h"

static int MBUF_ID = -1;

class MessageBroker;
class System;
class Client;


// potentially make two subclasses header, payload and maybe signature
struct Message {
  
    Message() { PCREATE; }
    Message(int& ticket, std::string _url, std::string fromHost = "", std::string fromPort = "");
    Message(std::string response) {
        BBLU("Message::Message: Response size is: %li\n", response.size());
        size_t p;
        std::string headers_str;
        if ((p = response.find("\r\n\r\n")) != std::string::npos) {
            headers_str = response.substr(0, p);
            this->sent = response.substr(p+4, response.size() - p - 4);
        }
        if ((p = headers_str.find("\r\n")) != std::string::npos) {
            this->directive = headers_str.substr(0, p);
            headers_str = headers_str.substr(p+2, headers_str.size() - p - 2);
        }
        while ((p = headers_str.find("\r\n")) != std::string::npos) {
            std::string keyval = headers_str.substr(0, p);
            size_t p2;
            if ((p2 = keyval.find(": ")) != std::string::npos) {
                headers[keyval.substr(0, p2)] = keyval.substr(p2+2, keyval.size()-p-2);
            }
            headers_str = headers_str.substr(p+2, headers_str.size()-p-2);
        }
        if (!headers_str.empty()) {
            std::string keyval = headers_str.substr(0, p);
            size_t p2;
            if ((p2 = keyval.find(": ")) != std::string::npos) {
                headers[keyval.substr(0, p2)] = keyval.substr(p2+2, keyval.size()-p-2);
            }
        }
    }

    Message(std::string src, std::string dest, const std::string& dir, const std::string& content) {
        PCREATE;
        BRED("Creating message buffer!\n");
        RED("%s\n", src.c_str());
        RED("%s\n", dest.c_str());
        RED("%s\n", dir.c_str());
        std::vector<std::string> toks;
        std::vector<std::string> toks2;
        std::vector<std::string> toks3;
        if (src.find(":") != std::string::npos) { toks = prizm::tokenize(src, ':'); }
        if (dest.find(":") != std::string::npos) { toks2 = prizm::tokenize(src, ':'); }
        if (toks2.size() >= 2) {
            if (src.find(":") != std::string::npos) { toks3 = prizm::tokenize(toks2[1], ':'); }
        }
        // toks2 = prizm::tokenize(dest, ':');
        // toks3 = prizm::tokenize(toks2[1], '/');
        BGRE("Successfully tokenized!\n");
        (toks2.size() >= 1) ? this->hostname = toks2[0] : this->hostname = "undefined";
        (toks3.size() >= 1) ? this->port = toks3[0] : this->port = "undefined";
        this->dir = dir;
        this->sent = content;
        (toks.size() >= 2) ? this->fromPort = toks[1] : this->port = "undefined";
        (toks3.size() >= 2) ? this->path = "/" + toks3[1] : this->path = "undefined";
        this->client = src;
        this->id = ++MBUF_ID;
    }

    ~Message() { BMAG("Deleting Message\n"); PDESTROY; }
  
    int ticket = -1;
    int id = -1;
    int modality = 0;
    int fulfilled = 0;
    double latency = 0.0;
    size_t size = 0;
    long timeout = 5000000;
    
    // for chunking
    bool chunked = false;
    size_t chunkSize = 2048;
    size_t fileSize;
    int chunkNum;
    bool simple = false;

    void chunk(std::string chunkStr, int chunkNum, size_t chunkSize, size_t fileSize) {
        this->chunkNum = chunkNum;
        this->chunkSize = chunkStr.size();
        this->fileSize = fileSize;
        this->chunked = true;
        if (chunkStr.size() > chunkSize) {
            BRED("Message::chunk: Size of chunk (%li) is greater than chunk limit (%li)!\n", chunkStr.size(), chunkSize);
            return;
        }
        this->sent = chunkStr;
        this->size = chunkStr.size();
    }

    void status(int statusCode);

    void setDirective(int status) {
        this->directive = HttpStatus::response(status);
    }

    std::string directive;

    std::string hostname;
    std::string port;

    std::string fromHost;
    std::string fromPort;
    std::string sent;

    std::string toHost;
    std::string toPort;
    std::string received = "undefined";

    std::string method = "GET";
    
    int statusCode = 100;
    std::string statusName;

    std::string client;
    std::string url;
    std::unordered_map<std::string, std::string> headers;

    std::string protocol = "HTTP/1.1";
    std::string path;
    std::string dir;
    std::string type = "text/plain";

    std::string flag = "undefined";
    std::string flag2 = "undefined";

    std::chrono::high_resolution_clock::time_point timestamp = std::chrono::high_resolution_clock::now();

    pthread_barrier_t* barrier = nullptr;
    std::weak_ptr<MessageBroker> broker;
  
    void publish();
  
    void mark();
  
    void dump();

    void callback(Callback* callback) {
        callback->serialize(this->headers);
    }

    std::string serialize();

    std::string serialize_directive();

    std::string serialize_simple() {
        std::string result;
        std::string protocol;
        if (this->protocol == "https") protocol = "HTTP/1.1";
        result += this->method + " " + this->path + " " + protocol + "\r\n";
        for (auto head : headers) {
            result += head.first + ": " + head.second + "\r\n";
        }
        result += "\r\n";
        result += this->sent;
        BYEL("Message::serialize_simple:\n");
        YEL("\t%s\n", result.c_str());
        return result;
    }

    std::string header(std::string key) {
        if (prizm::contains_key(headers, key)) {
            return headers[key];
        }
        return "";
    }

};

typedef std::string (*MessageCallback)(System* router, Client* client, std::deque<Message*>, std::string, void*);

#endif