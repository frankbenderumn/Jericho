#include "prizm2/prizm.h"

class CookieUnit : public ITest {
    MTRACE(CookieUnit);
  protected:
    void precondition() {
    }

    void postcondition() {
    }

    int a = 0;
    int b = 2;
};

PTEST(CookieUnit, Something) {
    PASSERT(a, 0);
    PBENCH(Okay);
    PSTOP(Okay);
}

// PTEST(CookieUnit, Generic) {
//     Cookie* cookie = new Cookie;
//     cookie->secure = true;
//     auto now = Clock::to_chrono(fixed);
//     now += Min(15);
//     cookie->expiration = Clock::sec(now);
//     cookie->token("scooby", "snacks");
//     std::string cook = cookie->generate();
//     // BYEL("Cookie: %s\n", cook.c_str());
//     PASSERT_STR(r1, cook);
//     delete cookie;
// }

// PTEST(CookieUnit, Multiple) {
//     Cookie* cookie = new Cookie;
//     cookie->secure = true;
//     auto now = Clock::to_chrono(fixed);
//     now += Min(15);
//     cookie->expiration = Clock::sec(now);
//     cookie->token("scooby", "snacks");
//     cookie->token("sessionId", "12klj987asd134hjasd9");
//     std::string cook = cookie->generate();
//     // BYEL("Cookie: %s\n", cook.c_str());
//     PASSERT_STR(r2, cook);
//     delete cookie;
// }

// PTEST(CookieUnit, Extras) {
//     Cookie* cookie = new Cookie;
//     cookie->secure = true;
//     auto now = Clock::to_chrono(fixed);
//     now += Min(15);
//     cookie->expiration = Clock::sec(now);
//     cookie->token("scooby", "snacks");
//     cookie->path = "/";
//     cookie->domain = "https://localhost";
//     cookie->maxAge = 60;
//     std::string cook = cookie->generate();
//     // BYEL("Cookie: %s\n", cook.c_str());
//     PASSERT_STR(r3, cook);
//     delete cookie;
// }

// PTEST(CookieUnit, Prefix) {
//     Cookie* cookie = new Cookie;
//     cookie->secure = true;
//     auto now = Clock::to_chrono(fixed);
//     now += Sec(5);
//     cookie->expiration = Clock::sec(now);
//     cookie->token("scooby", "snacks", COOKIE_PREFIX_HOST);
//     cookie->token("get", "some", COOKIE_PREFIX_SECURE);
//     cookie->sameSite = COOKIE_SAME_SITE_STRICT;
//     std::string cook = cookie->generate();
//     // BYEL("Cookie: %s\n", cook.c_str());
//     PASSERT_STR(r4, cook);
//     delete cookie;
// }

// PTEST(CookieUnit, Expired) {
//     Cookie* cookie = new Cookie;
//     cookie->secure = true;
//     auto now = Clock::to_chrono(fixed);
//     now += Sec(5);
//     cookie->expiration = Clock::sec(now);
//     cookie->token("scooby", "snacks");
//     std::string cook = cookie->generate();
//     // BYEL("Cookie: %s\n", cook.c_str());
//     PASSERT_STR(r5, cook);
//     delete cookie;
// }

// PTEST(CookieUnit, Unsecure) {
//     Cookie* cookie = new Cookie;
//     cookie->secure = false;
//     cookie->token("scooby", "snacks");
//     std::string cook = cookie->generate();
//     // BYEL("Cookie: %s\n", cook.c_str());
//     PASSERT_STR(r6, cook);
//     delete cookie;
// }