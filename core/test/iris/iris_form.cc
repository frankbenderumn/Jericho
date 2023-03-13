#include "prizm/prizm.h"
#include "util/trace.h"
#include "iris/iris.h"

std::string PUBLIC_DIRECTORY = "./core/test/data";

int main(int argc, char* argv[]) {
    TRACEMEM

    PrizmInit("Iris Form");

    TEST(IrisForm, Parse) {
        std::string file = "./core/test/data/iris/main.iris";
        std::string result = iris::interpret(nullptr, file);
        BMAG("Result: %s\n", result.c_str());
        ASSERT(1, 1);
    }

    PrizmResults();
    PrizmCleanup();

    return 0;
}