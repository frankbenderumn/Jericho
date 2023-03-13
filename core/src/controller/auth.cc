#include "controller/auth.h"
#include "util/clock.h"
#include "system/system.h"

namespace oauth {
    Response* login(System* system, Request* req, jericho::Session* sesh) {
        Response* res = NULL;

        std::string username = req->arg("Username");
        std::string password = req->arg("Password");

        BMAG("Username: %s\n", username.c_str());
        BMAG("Password: %s\n", password.c_str());

        if (username == "joeybenz300" && password == "pass1234") {
            req->path = "/staging/dashboard.iris";
            res = new Response(system, req);
            Cookie* cookie = new Cookie;
            cookie->secure = true;
            auto now = Clock::now_chrono();
            now += Min(1);
            cookie->expiration = Clock::sec(now);
            // cookie->token("scooby", "snacks");
            cookie->token("sessionId", "12klj987asd134hjase9", COOKIE_PREFIX_HOST);
            cookie->httpOnly = true;
            cookie->sameSite = COOKIE_SAME_SITE_LAX;
            std::string cook = cookie->generate();
            res->cookies.push_back(cookie);
            system->bifrost()->sessionManager()->create(req);
        } else if (sesh) {
            BGRE("Controller::auth: Valid session detected!\n");
            req->path = "/staging/dashboard.iris";
            res = new Response(system, req);
            Cookie* cookie = new Cookie;
            cookie->secure = true;
            auto now = Clock::now_chrono();
            now += Min(1);
            cookie->expiration = Clock::sec(now);
            // cookie->token("scooby", "snacks");
            cookie->token("sessionId", "12klj987asd134hjase9", COOKIE_PREFIX_HOST);
            cookie->httpOnly = true;
            cookie->sameSite = COOKIE_SAME_SITE_LAX;
            std::string cook = cookie->generate();
            res->cookies.push_back(cookie);
        }

        return res;
    }
}