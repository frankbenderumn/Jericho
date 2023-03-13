#include "prizm/prizm.h"
#include "iris/iris.h"
#include "util/file_system.hpp"

std::string PUBLIC_DIRECTORY = "./core/test/data";

int main(int argc, char* argv[]) {
    PrizmInit("Iris interpreter");

    std::string file = "./public/frontend/jericho/user.iris";

    // std::string file_contents = JFS::read(file);

    std::string result = iris::interpret(nullptr, file);

    BMAG("Result: %s\n", result.c_str());

    PrizmResults();
    PrizmCleanup();
    return 0;
}