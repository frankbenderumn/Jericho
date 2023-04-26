/* 
  Frank Bender
  Prizm Software Testing
*/

#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <string>
#include <regex>
#include <vector>

#include "picojson.h"

// #include "util/error.h"
namespace Jericho {
  class FileSystem {
    public:
      static void write(const char* path, std::string toWrite, bool overwrite = false);

      static void write(const char* path, const char* _toWrite, bool overwrite = false);

      static std::string read(const char* path);

      static int exists(std::string pathname);

      static std::vector<std::string> getDir(std::string name);

      static void mkDir(std::string name);

      static bool dirExists(std::string pathname);

      static bool fileExists(std::string pathname);

      bool static sanitize(std::string s, std::regex r);

      static std::string readBinary(const char* path);

      static void writeBinary(const char* path, std::string content, bool overwrite = false);

      static void readCBinary(const char* path, unsigned char* buffer);

      static void writeCBinary(const char* path, unsigned char* buffer);

      static int readJson(picojson::value& val, const char* path);

      static int parseJson(picojson::value& val, const char* path);

      static long modifiedAt(const char* path);

  };
}

typedef Jericho::FileSystem JFS;

#endif