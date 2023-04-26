#ifndef JERICHO_UTIL_LOGGER_H_
#define JERICHO_UTIL_LOGGER_H_

#include "util/file_system.hpp"

void writeLog(const char* path, const char* content) {
    JFS::write(path, content);
}

#define LOGGER(port, content) \
    std::string path = "./log/" + port + ".node"; \
    writeLog(path.c_str(), content);

#endif