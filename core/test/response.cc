#include "message/bifrost.h"
#include "server/response.h"
#include "system/system.h"
#include "prizm/prizm.h"
#include "util/clock.h"

std::string PUBLIC_DIRECTORY = "./core/test/data";

int main(int argc, char* argv[]) {
    TRACEMEM
    std::string r1 = JFS::read("./core/test/data/test_request.txt");
    // fixtures
    Client* clients = 0;
    std::string rPost = JFS::read("./core/test/data/request_post.txt");
    Client* cli = create_client(&clients);
    strncpy(cli->request, r1.c_str(), r1.size());
    Request* req = new Request(cli, MAX_REQUEST_SIZE, true);
    req->eval();

    ThreadPool* tpool = thread_pool_create((size_t)5);
    System* sys = new System(tpool, fetch);
    sys->router()->bind(ROUTE_HTTP, "/test.html", NULL);

    sys->router()->dump();

    req->dump();

    std::string result = sys->router()->exec(ROUTE_HTTP, req->path, req, sys, cli);
    BYEL("Result is: %s\n", result.c_str());
    
    Response* res = new Response(sys, req);

    time_t fixed = 1675209963;

    Cookie* cookie = new Cookie;
    cookie->secure = true;
    auto now = Clock::to_chrono(fixed);
    now += Min(15);
    cookie->expiration = Clock::sec(now);
    cookie->token("scooby", "snacks");
    cookie->token("sessionId", "12klj987asd134hjasd9");
    std::string cook = cookie->generate();
    res->cookies.push_back(cookie);

    std::string response = res->generate();

    BMAG("Response:\n%s\n", response.c_str());
    
    PrizmInit("Response");
    TEST(Response, Generic) {
        // Response* resp = new Response();
    }
    PrizmResults();
    PrizmCleanup();
    return 0;
}