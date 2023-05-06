#ifndef JERICHO_UTIL_TOOLS_H_
#define JERICHO_UTIL_TOOLS_H_

#include <sstream>
#include <string>
#include <iomanip>

std::string size_to_hex(size_t size);

bool parseInt(int& val, const std::string& value);

bool parseLong(long& val, const std::string& value);

bool parseSize(size_t& val, const std::string& value);

#endif