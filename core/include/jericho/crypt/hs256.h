#ifndef CRYPT_HS256_H_

#include <string>

#include "crypt/base64.h"

namespace jcrypt {

    namespace hs256 {

        std::string sign(const std::string& message, const std::string& secret) {
            DEBUG("HS256 SIGN:\n");
            DEBUG("\t\033[0;35mSecret\033[0m: %s (%li)\n", secret.c_str(), secret.size());
            DEBUG("\t\033[0;35mToken to sign\033[0m:\n%s (%li)\n", message.c_str(), message.size());
            unsigned char signature[EVP_MAX_MD_SIZE];
            unsigned int signature_len = 0;
            HMAC(EVP_sha256(), secret.c_str(), secret.length(),
                reinterpret_cast<const unsigned char*>(message.c_str()), message.size(),
                signature, &signature_len);
            std::string h(reinterpret_cast<const char*>(signature), signature_len);
            std::string encoded_signature = jcrypt::base64::encode_url(h);
            // url_safe(encoded_signature);
            return encoded_signature;
        }

    }

}

#endif