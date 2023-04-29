#ifndef JERICHO_UTIL_TOOLS_H_
#define JERICHO_UTIL_TOOLS_H_

#include <sstream>
#include <string>
#include <iomanip>

std::string size_to_hex(size_t size) {
    std::stringstream stream;
    stream << std::hex << size;
    return stream.str();
}