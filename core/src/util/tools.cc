#include "util/tools.h"

std::string size_to_hex(size_t size) {
    std::stringstream stream;
    stream << std::hex << size;
    return stream.str();
}

bool parseInt(int& val, const std::string& value) {
    if (value.empty()) return false;
    for (auto& c : value) {
        if (!std::isdigit(c)) return false;
    }
    val = std::stoi(value);
    return true;
}

bool parseLong(long& val, const std::string& value) {
    if (value.empty()) return false;
    for (auto& c : value) {
        if (!std::isdigit(c)) return false;
    }
    val = std::stol(value);
    return true;    
}

bool parseSize(size_t& val, const std::string& value) {
    if (value.empty()) return false;
    for (auto& c : value) {
        if (!std::isdigit(c)) return false;
    }
    val = std::stoull(value);
    return true;    
}