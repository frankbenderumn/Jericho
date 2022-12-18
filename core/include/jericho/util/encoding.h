#include "server/defs.h"

#include <locale>

int UTF8_to_ascii(unsigned char* out, int *outlen,
              const unsigned char* in, int *inlen);

std::string from_utf8(const std::string& str, const std::locale& loc = std::locale{});
