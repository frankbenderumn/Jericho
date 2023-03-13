#include "util/trace.h"

int main (int argc, char* argv[]) {
    PrizmInit("String Util");

    TEST(String, substrcmp) {
        ASSERT(1, 1);
    }

    PrizmResults();
    PrizmCleanup();
    return 0;
}