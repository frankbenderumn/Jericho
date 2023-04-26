#ifndef JERICHO_UTIL_URL_H_
#define JERICHO_UTIL_URL_H_

#include <string>
#include <unordered_map>
#include "prizm/prizm.h"
#include "util/trace.h"

class URL {
    void _parseProtocol(std::string protocol) {
        std::string::size_type p;
        if ((p = protocol.find("+")) != std::string::npos) {
            this->type = protocol.substr(0, p);
            this->protocol = protocol.substr(p+1, protocol.size() - p - 1); 
        }
    }

    void _parseHostname(std::string hostname) {
        std::string::size_type p;
        if ((p = hostname.find(":")) != std::string::npos) {
            host = hostname.substr(0, p);
            port = hostname.substr(p+1, hostname.size() - p - 1);
            // BMAG("Host: %s\n", host.c_str());
            // BMAG("Port: %s\n", port.c_str());
            ip = true;
        } else if ((p = hostname.find(".")) != std::string::npos) {
            p = 0;
            std::vector<std::string> tokens;
            while ((p = hostname.find(".")) != std::string::npos) {
                tokens.push_back(hostname.substr(0, p));
                hostname = hostname.substr(p+1, hostname.size() - p - 1);
            }
            tokens.push_back(hostname);
            for (auto tok : tokens) {
                // BBLU("Token: %s\n", tok.c_str());
            }

            if (tokens.size() == 2) {
                this->ext = tokens[1];
                this->hostname = tokens[0];
            } else if (tokens.size() > 2) {
                this->ext = tokens[tokens.size() - 1];
                this->hostname = tokens[tokens.size() - 2];
                for (int i = 0; i < tokens.size() - 2; i++) {
                    this->subdomains.push_back(tokens[i]);
                }
            }
            domain = true;

        } else if (this->protocol != "jtp") {
            valid = false;
        }
        valid = false;
    }

    void _parsePath(std::string path) {
        char* dot = strstr((char*)path.c_str(), ".");
        char* dotdot = strstr((char*)path.c_str(), ".");
        if (dot != nullptr || dotdot != nullptr) {
            BRED("Invalid path detected!\n");
            this->valid = false;
        }
    }

    void _parseArgs(std::string arglist) {
        std::vector<std::pair<std::string, std::string>> args = {};
        char* p = (char*)arglist.c_str();
        int len = strlen(p);
        char* p2, *p3;
        while ((p2 = strstr(p, "&")) != NULL) {
            p3 = strstr(p, "=");
            if (p3 != nullptr) {
                std::string arg = std::string(p, p3 - p);
                std::string val = std::string(p3 + 1, p2 - p3 - 1);
                // BYEL("ARG: %s\n", arg.c_str());
                // BYEL("VAL: %s\n", val.c_str());
                args.push_back({arg, val});
            }
            p = p2 + 1;
        }
        if ((p2 = strstr(p, "&")) == NULL) {
            p2 = strstr(p, "=");
            if (p2 != nullptr) {
                std::string arg = std::string(p, p2 - p);
                len -= p2 - p - 1;
                std::string val = std::string(p2 + 1, len);
                // BYEL("ARG: %s\n", arg.c_str());
                // BYEL("VAL: %s\n", val.c_str());
                args.push_back({arg, val});
            }
        }
        for (auto arg : args) {
            _args[arg.first] = arg.second;
        }
        // assign args
    }

    bool _urlSafe(std::string str) {
        return true;
    }

    std::unordered_map<std::string, std::string> _args;

  public:
    std::string protocol;
    std::vector<std::string> subdomains;
    std::string host;
    std::string hostname;
    std::string ext;
    std::string path;
    std::string port;
    std::string url;
    std::string type = "plain";
    bool valid = true;
    bool domain = false;
    bool ip = false;

    URL() { PCREATE; }

    URL(std::string url) {
        PCREATE;
        this->url = url;
        char* p = NULL, *p2 = NULL, *p3 = NULL;
        const char* str = url.c_str();
        int len = strlen(str);
        // BRED("len: %i\n", len);
        p = strstr((char*)str, "://");
        if (!p) { 
            // BRED("URL does not provide a protocol!\n");
        }
        std::string s(p);
        protocol = std::string(str, p - str);
        // BYEL("Protocol is: %s\n", protocol.c_str());
        len -= protocol.size() + 3;
        p += 3;
        bool no_path = false;
        // BRED("len: %i\n", len);
        p2 = strstr((char*)p, "/");
        if (!p2) { 
            // BRED("URL does not provide a path!\n"); 
            hostname = std::string(p, len);
            no_path = true;
        } else {
            hostname = std::string(p, p2 - p);
        }
        // BYEL("Hostname is: %s\n", hostname.c_str());
        p += (p2 - p);
        len -= hostname.size();
        // BRED("len: %i\n", len);
        std::string args;
        if (!no_path) {
            p3 = strstr((char*)p, "?");
            if (!p3) {
                // BRED("No arguments detected!\n"); 
                path = std::string(p2, len);        
            } else {
                p3 += 1;
                path = std::string(p, p3 - p - 1);
                len -= path.size() + 1;
                // BRED("len: %i\n", len);
                args = std::string(p3, len);
            }
            // BYEL("Path is: %s\n", path.c_str());
            // BYEL("Args are: %s\n", args.c_str());
        }
        _parseProtocol(protocol);
        _parseHostname(hostname);
        if (!path.empty()) {
            _parsePath(path);
        } else {
            path = "/";
        }
        if (!args.empty()) _parseArgs(args);
    }

    ~URL() { PDESTROY; }

    bool arg(std::string& value, std::string key) {
        if (prizm::contains_key(_args, key)) {
            value = _args[key];
            return true;
        }
        return false;
    }
};

#endif