#include "prizm2/prizm.h"
#include "session/cookie.h"

class CookieUnit : public ITest {
    MTRACE(CookieUnit);
  protected:
    void precondition() {
    }

    void postcondition() {
    }

    std::string r1 = "scooby=snacks; Expires=Wed, 01 Feb 2023 00:21:03 GMT; Secure; HttpOnly; SameSite=Lax;";
    std::string r2 = "sessionId=12klj987asd134hjasd9; scooby=snacks; Expires=Wed, 01 Feb 2023 00:21:03 GMT; Secure; HttpOnly; SameSite=Lax;";
    std::string r3 = "scooby=snacks; Expires=Wed, 01 Feb 2023 00:21:03 GMT; Secure; HttpOnly; SameSite=Lax; Domain=https://localhost; Path=/; Max-Age=60;";   
    std::string r4 = "__Secure-get=some; __Host-scooby=snacks; Path=/; Expires=Wed, 01 Feb 2023 00:06:08 GMT; Secure; HttpOnly; SameSite=Strict;";
    std::string r5 = "scooby=snacks; Expires=Wed, 01 Feb 2023 00:06:08 GMT; Secure; HttpOnly; SameSite=Lax;";
    std::string r6 = "scooby=snacks; HttpOnly; SameSite=Lax;";

    // Pretty time: Tue, Jan 31, 2023 06:06:03 PM CST
    time_t fixed = 1675209963;

};

PTEST(CookieUnit, Generic) {
    Cookie* cookie = new Cookie;
    cookie->secure = true;
    auto now = Clock::to_chrono(fixed);
    now += Min(15);
    cookie->expiration = Clock::sec(now);
    cookie->token("scooby", "snacks");
    std::string cook = cookie->generate();
    // BYEL("Cookie: %s\n", cook.c_str());
    PASSERT(r1, cook);
    delete cookie;
}

PTEST(CookieUnit, Multiple) {
    Cookie* cookie = new Cookie;
    cookie->secure = true;
    auto now = Clock::to_chrono(fixed);
    now += Min(15);
    cookie->expiration = Clock::sec(now);
    cookie->token("scooby", "snacks");
    cookie->token("sessionId", "12klj987asd134hjasd9");
    std::string cook = cookie->generate();
    // BYEL("Cookie: %s\n", cook.c_str());
    PASSERT(r2, cook);
    delete cookie;
}

PTEST(CookieUnit, Extras) {
    Cookie* cookie = new Cookie;
    cookie->secure = true;
    auto now = Clock::to_chrono(fixed);
    now += Min(15);
    cookie->expiration = Clock::sec(now);
    cookie->token("scooby", "snacks");
    cookie->path = "/";
    cookie->domain = "https://localhost";
    cookie->maxAge = 60;
    std::string cook = cookie->generate();
    // BYEL("Cookie: %s\n", cook.c_str());
    PASSERT(r3, cook);
    delete cookie;
}

PTEST(CookieUnit, Prefix) {
    Cookie* cookie = new Cookie;
    cookie->secure = true;
    auto now = Clock::to_chrono(fixed);
    now += Sec(5);
    cookie->expiration = Clock::sec(now);
    cookie->token("scooby", "snacks", COOKIE_PREFIX_HOST);
    cookie->token("get", "some", COOKIE_PREFIX_SECURE);
    cookie->sameSite = COOKIE_SAME_SITE_STRICT;
    std::string cook = cookie->generate();
    // BYEL("Cookie: %s\n", cook.c_str());
    PASSERT(r4, cook);
    delete cookie;
}

PTEST(CookieUnit, Expired) {
    Cookie* cookie = new Cookie;
    cookie->secure = true;
    auto now = Clock::to_chrono(fixed);
    now += Sec(5);
    cookie->expiration = Clock::sec(now);
    cookie->token("scooby", "snacks");
    std::string cook = cookie->generate();
    // BYEL("Cookie: %s\n", cook.c_str());
    PASSERT(r5, cook);
    delete cookie;
}

PTEST(CookieUnit, Unsecure) {
    Cookie* cookie = new Cookie;
    cookie->secure = false;
    cookie->token("scooby", "snacks");
    std::string cook = cookie->generate();
    // BYEL("Cookie: %s\n", cook.c_str());
    PASSERT(r6, cook);
    delete cookie;
}