/* 
  Frank Bender
  Prizm Software Testing
*/

#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <string>
#include <regex>
#include <vector>
// #include "util/error.h"
namespace Jericho {
  class FileSystem {
    public:
      static void write(const char* path, std::string toWrite);

      static std::string read(const char* path);

      static std::vector<std::string> getDir(std::string name);

      bool static sanitize(std::string s, std::regex r);

      static std::string readBinary(const char* path);

      static void writeBinary(const char* path, std::string content);

      static void readCBinary(const char* path, unsigned char* buffer);

      static void writeCBinary(const char* path, unsigned char* buffer);
  };
}

#endif