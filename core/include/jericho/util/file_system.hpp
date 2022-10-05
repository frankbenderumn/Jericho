/* 
  Frank Bender
  Prizm Software Testing
*/

#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <string>
#include <regex>
// #include "util/error.h"
namespace Jericho {
  class FileSystem {
    public:
      static void write(const char* path, std::string toWrite);

      static std::string read(const char* path);

      static std::string getDir(std::string name);

      bool static sanitize(std::string s, std::regex r);
  };
}

#endif