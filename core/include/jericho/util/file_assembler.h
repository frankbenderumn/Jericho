#ifndef JERICHO_UTIL_FILE_ASSEMBLER_H_
#define JERICHO_UTIL_FILE_ASSEMBLER_H_

#include <unordered_map>

#include "util/trace.h"
#include "server/request.h"

class FileAssembler {
    std::map<int, Request*> chunks;
  public:
    FileAssembler(Request* req) {
        PCREATE;
        _chunks[0] = req;
    }

    ~FileAssembler() { PDESTROY; }

    void assemble() {
        for (auto& chunk : chunks) {
            delete chunk.second;
        }
    }

    void addChunk(Request* req) {
        if (!req->final_chunk) {
            _chunks[req->chunk] = req;
        } else {
            this->assemble;
        }
    }

};

#endif