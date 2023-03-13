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

typedef std::chrono::high_resolution_clock hrclock;

struct Request {
    std::string content;
    std::unordered_map<std::string, std::string> headers;
    std::string method;
    std::string path;
    std::string request;
    std::unordered_map<std::string, std::string> args;
    std::string protocol;
    std::string signature;
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
    ~Request() { PDESTROY; }

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
        BYEL("method: %s\n", this->method.c_str());
        BYEL("path: %s\n", this->path.c_str());
        BYEL("content: %s\n", this->content.c_str());
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
                    this->request = std::string(client->request);
                    all_read = true;
                } else {
                    BRED("Request::poll_request: Http headers not found!\n");
                    RED("%s\n", client->request);
                }
                client->received += strlen(client->request) + 1;
                // BMAG("Request::poll_request: Number of bytes received from client is %i\n", client->received);
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

    void eval() {
        std::string bytes(client->request);
        // BYEL("Request::eval example:\n");
        // printf("%s\n", bytes.c_str());

        std::string::size_type p = bytes.find("\r\n\r\n");
        if (p != std::string::npos) {
            // BBLU("Term location: %li :: %li\n", p, bytes.size());
        } else {
            BRED("Request::eval: Invalid headers (\\r\\n\\r\\n not found)\n");
            this->valid = false;
        }

        std::string headersStr;
        std::string content;

        if (p != bytes.size() - 4) {
            size_t last = 0;
            size_t next = 0;
            std::vector<std::string> result;
            std::string delim = "\r\n\r\n";
            while ((next = bytes.find(delim, last)) != std::string::npos) {
                if (bytes.substr(last, next-last) != "") {
                    result.push_back(bytes.substr(last, next-last));
                }
                last = next + delim.size();
            }
            if (bytes.substr(last, next-last) != "") {
                result.push_back(bytes.substr(last));
            }
            if (result.size() == 2) {
                // BWHI("Headers:\n");
                // printf("%s\n", result[0].c_str());
                headersStr = result[0];
                // BWHI("Content:\n");
                // printf("%s\n", result[1].c_str());
                this->content = result[1];
            } else if (result.size() == 1) {
                BWHI("Request::eval: Headers only!\n");
            }
        } else {
            headersStr = bytes;
        }

        std::istringstream ss(headersStr);
        std::string word;
        std::vector<std::string> words;
        bool first = true;
        std::string protoHead;
        std::unordered_map<std::string, std::string> args2;
        while (std::getline(ss, word, '\n')) {
            if (word != "") {
                std::string::size_type sep = word.find(": ");
                if (sep != std::string::npos && !first) {
                    std::string key = word.substr(0, sep);
                    std::string val = word.substr(sep+2, word.size() - sep+2);
                    // if (val[val.size() - 1] == '\r') {
                    //     val.pop_back();
                    // }
                    prizm::erase(val, '\r');
                    args2[key] = val;
                } else if (first) {
                    protoHead = word;
                    first = !first;
                }
                words.push_back(word);
            }
        }
        // BBLU("ProtoHead:\n");
        // printf("%s\n", protoHead.c_str());
        int sepGate = 0;
        std::string method2;
        std::string path2;
        std::string protocol2;
        for (auto c : protoHead) {
            if (c == ' ') { sepGate++; continue; }
            if (c == '\t') { continue; }
            if (c == '\r') { continue; }
            if (sepGate == 0) { method2 += c; }
            else if (sepGate == 1) { path2 += c; }
            else if (sepGate == 2) { protocol2 += c; }
        }
        this->method = method2;
        this->path = path2;
        this->protocol = protocol2;
        // BBLU("Method\n");
        // printf("%s\n", method2.c_str());
        // BBLU("Path\n");
        // printf("%s\n", this->path.c_str());
        // BBLU("Protocol\n");
        // printf("%s\n", protocol2.c_str());
        // BBLU("Headers:\n");
        // for (auto a : args2) {
        //     printf("%s -- %s\n", a.first.c_str(), a.second.c_str());
        // }
        this->headers = args2;

        size_t pathp = this->path.find("?");
        if (pathp != std::string::npos) {
            std::string npath = this->path.substr(pathp+1, this->path.size());
            this->path = this->path.substr(0, pathp);
            std::vector<std::string> toks = prizm::tokenize(npath, '&');
            for (auto t : toks) {
                std::string::size_type p = t.find("=");
                if (p != std::string::npos) {
                    std::string key = t.substr(0, p);
                    std::string val = t.substr(p+1, t.size());
                    args[key] = val;
                }
            }
        }

        if (this->method == "POST" && this->content != "") {
            std::vector<std::string> toks = prizm::tokenize(this->content, '&');
            for (auto t : toks) {
                std::string::size_type p = t.find("=");
                if (p != std::string::npos) {
                    std::string key = t.substr(0, p);
                    std::string val = t.substr(p+1, t.size());
                    args[key] = val;
                }
            }
        }
    }

    std::string header(std::string key) {
        if (prizm::contains_key(this->headers, key)) {
            return this->headers[key];
        } else {
            BRED("Request::header: Header %s does not exist!\n", key.c_str());
        }
        return "undefined";
    }

    std::string arg(std::string key) {
        if (prizm::contains_key(this->args, key)) {
            return this->args[key];
        } else {
            BRED("Request::arg: Arg %s does not exist!\n", key.c_str());
        }
        return "undefined";
    }
};

// std::vector<std::string> prizm::tokenize(std::string in, std::string delim) {
//     size_t last = 0;
//     size_t next = 0;
//     std::vector<std::string> result = {};
//     while ((next = in.find(delim, last)) != std::string::npos) {
//         if (in.substr(last, next-last) != "") {
//             result.push_back(in.substr(last, next-last));
//         }
//         last = next + delim.size();
//     }
//     if (in.substr(last, next-last) != "") {
//         result.push_back(in.substr(last));
//     }
//     return result;
//     }
// };

bool is_valid_request(Client* client);

int parse_request(Client* client, Request* request);

void print_request(Request* request);

bool isHTTP(std::string request);

#endif