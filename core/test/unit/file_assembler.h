#include "util/file_assembler.h"
#include "prizm/prizm2.h"

class FileAssemblerUnit : ITest {
    MTRACE(FileAssemblerUnit);
  protected:
    void precondition() {}
    void postcondition() {}
    std::string chunk1 = "./test/data/chunked/chunk1.txt";
    std::string chunk2 = "./test/data/chunked/chunk2.txt";
    std::string chunk3 = "./test/data/chunked/chunk3.txt";
};

PTEST(FileAssemblerUnit, chunking_test) {

}