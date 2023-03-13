#include "prizm/prizm.h"
#include "server/defs.h"

std::string PUBLIC_DIRECTORY = "./public/frontend";

void atexit_fish() {
    TRACESCAN;
}

int main(int argc, char* argv[]) {
    const int result_1 = std::atexit(atexit_fish);

    PrizmInit("Tracing");
    TraceTest test;
    TraceTest test2;
    TraceTest test3;
    ExampleTrace ex;
    SraceTest stest;
    BampleTrace bamp;
    BampleTracedd bampdd;
    AmpleTrace amp;
    AmpleTrace amp2;
    CampleTrace* camp = new CampleTrace;
    UTrace utrace;
    ExampleTrace* ex2 = new ExampleTrace;
    BampleTraced* bampd = new BampleTraced;
    AAmpleTrace aa;
    Srace srace;
    TracerTest trtest;
    AmppleTrace apple;
    TraceTests trtests;
    TRACEDUMP;
    PrizmResults();
    PrizmCleanup();
    return 0;
}