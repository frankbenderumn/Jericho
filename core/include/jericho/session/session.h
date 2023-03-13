#ifndef JERICHO_SESSION_SESSION_H_
#define JERICHO_SESSION_SESSION_H_

#include <chrono>

#include "util/trace.h"
#include "util/clock.h"
#include "server/defs.h"
// #include "server/client.h"
#include "server/request.h"

// typedef std::chrono::system_clock Clock;
// typedef std::chrono Chrono;

namespace jericho {

struct Session {
    Client* client;
    std::string addr;
    time_t expires_at;
    time_t created_at;
    std::string token;
    Cookie* cookie;
    Session() { PCREATE; }
    ~Session() { PDESTROY; }

    bool expired() {
        time_t now = Clock::now();
        if (expires_at - now > 0) return false;
        return true;
    }
};

class SessionManager {
    std::vector<Session*> _sessions;
  public:
    SessionManager() { PCREATE; }
    ~SessionManager() { 
        for (auto sesh : _sessions) {
            delete sesh;
        }
        PDESTROY;
    }
    void create(Request* req) {
        Session* sesh = new Session;
        sesh->client = req->client;
        auto now = Clock::now_chrono();
        auto exp = now + Min(10);
        // char address_buffer[16];
        // client_get_address(req->client, address_buffer);
        // std::string addr(address_buffer);
        std::string addr = "127.0.0.1";
        sesh->addr = addr;
        sesh->created_at = Clock::sec(now);
        sesh->expires_at = Clock::sec(exp);
        std::string pret1 = Clock::prettyDate(sesh->created_at);
        std::string pret2 = Clock::prettyDate(sesh->expires_at);
        GRE("Session created at ");
        BWHI("%s\n", pret1.c_str());
        GRE("Session expires at ");
        BWHI("%s\n", pret2.c_str());
        _sessions.push_back(sesh);
    }

    const size_t sessionCount() const { return _sessions.size(); }

    bool sessionValid(Request* req) {
        int i = 0;
        for (auto sesh : _sessions) {
            // char address_buffer[16];
	        // client_get_address(req->client, address_buffer);
            // std::string addr(address_buffer);
            std::string addr = "127.0.0.1";
            if (addr == sesh->addr) {
                BBLU("Session found!\n");
                if (!sesh->expired()) {
                    BGRE("Valid session!\n");
                    return true;
                } else {
                    BRED("Session Expired! Deleting...\n");
                    _sessions.erase(_sessions.begin() + i);
                    delete sesh;
                    break;
                }
            }
            i++;
        }
        return false;
    }

    Session* verify(Cookie* cookie) {
        int i = 0;
        for (auto sesh : _sessions) {
            // char address_buffer[16];
	        // client_get_address(req->client, address_buffer);
            // std::string addr(address_buffer);
            std::string addr = "127.0.0.1";
            if (cookie->addr == sesh->addr) {
                BBLU("Session found!\n");
                if (!sesh->expired()) {
                    BGRE("Valid session!\n");
                    return sesh;
                } else {
                    BRED("Session Expired! Deleting...\n");
                    _sessions.erase(_sessions.begin() + i);
                    delete sesh;
                    break;
                }
            }
            i++;
        }
        return NULL;
    }
};

}

#endif