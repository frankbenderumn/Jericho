#include "session/cookie.h"
#include "prizm/prizm.h"
#include "util/trace.h"
#include "util/clock.h"

std::string PUBLIC_DIRECTORY = "./public/frontend";

// fixtures
std::string r1 = "scooby=snacks; Expires=Tue, 31 Jan 2023 18:21:03 CST; Secure; HttpOnly; SameSite=Lax;";
std::string r2 = "sessionId=12klj987asd134hjasd9; scooby=snacks; Expires=Tue, 31 Jan 2023 18:21:03 CST; Secure; HttpOnly; SameSite=Lax;";
std::string r3 = "scooby=snacks; Expires=Tue, 31 Jan 2023 18:21:03 CST; Secure; HttpOnly; SameSite=Lax; Domain=https://localhost; Path=/; Max-Age=60;";   
std::string r4 = "__Secure-get=some; __Host-scooby=snacks; Expires=Tue, 31 Jan 2023 18:06:08 CST; Secure; HttpOnly; SameSite=Strict;";
std::string r5 = "scooby=snacks; Expires=Tue, 31 Jan 2023 18:06:08 CST; Secure; HttpOnly; SameSite=Lax;";
std::string r6 = "scooby=snacks; HttpOnly; SameSite=Lax;";

int main(int argc, char* argv[]) {
    /** TODO: add fixture support to Testing library */

    // time_t t = std::time(NULL);
    // BMAG("Time: %li\n", t);
    // BMAG("Pretty time: %s\n", Clock::prettyDate(t).c_str());
    TRACEMEM
    PrizmInit("Cookie");
    TEST(Cookie, Generic) {
        Cookie* cookie = new Cookie;
        cookie->secure = true;
        auto now = Clock::to_chrono(fixed);
        now += Min(15);
        cookie->expiration = Clock::sec(now);
        cookie->token("scooby", "snacks");
        std::string cook = cookie->generate();
        // BYEL("Cookie: %s\n", cook.c_str());
        ASSERT_STR(r1, cook);
        delete cookie;
    }

    TEST(Cookie, Multiple) {
        Cookie* cookie = new Cookie;
        cookie->secure = true;
        auto now = Clock::to_chrono(fixed);
        now += Min(15);
        cookie->expiration = Clock::sec(now);
        cookie->token("scooby", "snacks");
        cookie->token("sessionId", "12klj987asd134hjasd9");
        std::string cook = cookie->generate();
        // BYEL("Cookie: %s\n", cook.c_str());
        ASSERT_STR(r2, cook);
        delete cookie;
    }

    TEST(Cookie, Extras) {
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
        ASSERT_STR(r3, cook);
        delete cookie;
    }

    TEST(Cookie, Prefix) {
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
        ASSERT_STR(r4, cook);
        delete cookie;
    }

    TEST(Cookie, Expired) {
        Cookie* cookie = new Cookie;
        cookie->secure = true;
        auto now = Clock::to_chrono(fixed);
        now += Sec(5);
        cookie->expiration = Clock::sec(now);
        cookie->token("scooby", "snacks");
        std::string cook = cookie->generate();
        // BYEL("Cookie: %s\n", cook.c_str());
        ASSERT_STR(r5, cook);
        delete cookie;
    }

    TEST(Cookie, Unsecure) {
        Cookie* cookie = new Cookie;
        cookie->secure = false;
        cookie->token("scooby", "snacks");
        std::string cook = cookie->generate();
        // BYEL("Cookie: %s\n", cook.c_str());
        ASSERT_STR(r6, cook);
        delete cookie;
    }
    
    PrizmResults();
    PrizmCleanup();
    return 0;
}