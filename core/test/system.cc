#include "prizm/prizm.h"
#include "util/trace.h"
#include "system/system.h"
// #include "router/route_registry.h"

std::string PUBLIC_DIRECTORY = "./public/frontend";

int main(int argc, char* argv[]) {
    TRACEMEM

    PrizmInit("System");

    ThreadPool* tpool = thread_pool_create((size_t)5);

    System* sys = new System(tpool, fetch);

    TEST(System, construct) {
        ASSERT(1, 1);
    }

    PrizmResults();
    PrizmCleanup();

    return 0;
}
