// #define TEST_MODE

#include <string>

#include "prizm/prizm.h"
#include "util/trace.h"
#include "system/router.h"
#include "system/contracts.h"
// #include "router/route_registry.h"

std::string PUBLIC_DIRECTORY = "./public/frontend";

namespace test {

std::string routeFn(std::unordered_map<std::string, std::string> args) {
    return "something";
}

}

// benchmark code method:
// stop first?
// start next
// global array.
// for non threaded

// benchmark iter: took 0.141535 secs
// Result: something
// benchmark exec: took 0.000029 secs
// Result: something
// benchmark exec2: took 0.000550 secs
// Result: {"status": "500", "error": "Unregistered path detected for path: /example200000"}
// benchmark exec3: took 0.017375 secs

// use b-tree for more advanced indexing?
// trie-b-tree

int main(int argc, char* argv[]) {
    TRACEMEM;
    PrizmInit("Router");

    Router* router = new Router;

    // TEST(Router, BindHTTP) {
    //     Benchmark* bm = bm_start("iter");
    //     // for (int i = 0; i < 100000; i++) {
    //     //     std::string p = "/example" + std::to_string(i);
    //     //     router->bind(ROUTE_HTTP, p, test::routeFn, {pre}, {post});
    //     // }
    //     bm_stop(bm);
    //     // router->bind(ROUTE_HTTP, "/mordecai", NULL);
    //     // router->bind(ROUTE_API, "/members", NULL);
    //     // router->bind(ROUTE_SYSTEM, "/deploy", NULL);
    //     // router->dump();
    // }

    // TEST(Router, exec) {
    //     Benchmark* bm = bm_start("exec");
    //     std::string result = router->exec("/example200000", {}, NULL, NULL);
    //     BYEL("Result: %s\n", result.c_str());
    //     bm_stop(bm);
    // }    
    
    // // not sure if hasFunction is an exhaustive edge case check
    // // this way won't allow for design by contract
    // TEST(Router, exec2) {
    //     Benchmark* bm = bm_start("exec2");
    //     std::string result = router->exec2("/example200000", {}, NULL, NULL);
    //     BYEL("Result: %s\n", result.c_str());
    //     bm_stop(bm);
    // }

    // TEST(Router, exec3) {
    //     Benchmark* bm = bm_start("exec3");
    //     std::string result = router->exec3("/example200000", {}, NULL, NULL);
    //     BYEL("Result: %s\n", result.c_str());
    //     bm_stop(bm);
    // }

    // TEST(Router, InvalidContract) {
    //     Benchmark* bm = bm_start("invalid_contract");
    //     std::string result = router->exec("/example200", {}, NULL, NULL);
    //     BYEL("Result: %s\n", result.c_str());
    //     ASSERT_STR(result, "undefined");
    //     bm_stop(bm);
    // }

    // TEST(Router, ValidContract) {
    //     Benchmark* bm = bm_start("valid_contract");
    //     std::string result = router->exec("/example200", {{"Host", "127.0.0.1"}, {"Content", "Welcome"}}, NULL, NULL);
    //     BYEL("Result: %s\n", result.c_str());
    //     REFUTE_STR(result, "undefined");
    //     ASSERT_STR(result, "something");
    //     bm_stop(bm);
    // }

    delete router;

    PrizmResults();
    PrizmCleanup();

    return 0;
}
