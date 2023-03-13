#ifndef JERICHO_SESSION_COOKIE_H_
#define JERICHO_SESSION_COOKIE_H_

#include <unordered_map>
#include <string>

#include "prizm/prizm.h"
#include "util/trace.h"

enum CookieSameSite {
    COOKIE_SAME_SITE_NONE,
    COOKIE_SAME_SITE_LAX,
    COOKIE_SAME_SITE_STRICT
};

enum CookiePrefix {
    COOKIE_PREFIX_NONE,
    COOKIE_PREFIX_HOST,
    COOKIE_PREFIX_SECURE
};

// think about CHIPS support
struct Cookie {
    Cookie() { PCREATE; }
    ~Cookie() { PDESTROY; }
    Cookie(std::string addr, std::string key, std::string val) { 
        PCREATE;
        this->addr = addr;
        // this->key = key;
        // this->val = val;
        this->tokens[key] = val;
    }
    time_t expiration = 0;
    bool secure = true;
    bool httpOnly = true;
    CookieSameSite sameSite = COOKIE_SAME_SITE_LAX;
    CookiePrefix prefix = COOKIE_PREFIX_NONE;
    std::string path = "undefined";
    std::string domain = "undefined";
    // std::string key = "undefined";
    // std::string val = "undefined";
    std::unordered_map<std::string, std::string> tokens;
    std::string addr = "undefined";
    long maxAge = 0;
    std::string generate() {
        std::string cookie;
        for (auto tok : tokens) {
            cookie += tok.second + "; ";
        }
        if (expiration != 0) {
            char buf[50];

            strftime(buf, 50, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&expiration));
            std::string time(buf);
            cookie += "Expires=" + time + "; ";
        }
        if (secure) cookie += "Secure; ";
        if (httpOnly) cookie += "HttpOnly; ";
        switch (sameSite) {
            case COOKIE_SAME_SITE_LAX:
                cookie += "SameSite=Lax; ";
                break;
            case COOKIE_SAME_SITE_STRICT:
                cookie += "SameSite=Strict; ";
                break;
            case COOKIE_SAME_SITE_NONE:
                // cookie += "SameSite=None; ";
                break;
            default:
                break;
        }
        if (domain != "undefined") cookie += "Domain=" + domain + "; ";
        if (path != "undefined") cookie += "Path=" + path + "; ";
        if (maxAge != 0) cookie += "Max-Age=" + std::to_string(maxAge) + "; ";
        cookie.pop_back();
        return cookie; 
    }
    void token(std::string key, std::string val, CookiePrefix prefix = COOKIE_PREFIX_NONE) {
        switch (prefix) {
            case COOKIE_PREFIX_HOST:
                tokens[key] = "__Host-" + key + "=" + val + "; Path=/";
                break;
            case COOKIE_PREFIX_SECURE:
                tokens[key] = "__Secure-" + key + "=" + val;
                break;
            default:
                tokens[key] = key + "=" + val;
                break;
        }
    }
    const void dump() const {
        BBLU("Cookie\n=====================================================\n");
        // BLU("Key: %s\n", key.c_str());
        // BLU("Val: %s\n", val.c_str());
    }
};

#endif