#ifndef JERICHO_CRYPT_BASE64_H_
#define JERICHO_CRYPT_BASE64_H_

#include <string>
#include <algorithm>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

namespace jcrypt {

namespace base64 {

std::string encode(const std::string& input);
std::string decode(const std::string& input);

std::string encode_url(const std::string& input);
std::string decode_url(const std::string& input);

size_t size(const std::string& original);
size_t size_num(const size_t& original_sz);

}

}

#endif