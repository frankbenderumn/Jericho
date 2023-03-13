#ifndef JERICHO_CRYPT_CRYPT_H_
#define JERICHO_CRYPT_CRYPT_H_

#include <openssl/rand.h>
#include <openssl/err.h>

#include <string>

namespace jcrypt {

    std::string generate_session_token(int length) {
        unsigned char buffer[length];
        if (!RAND_bytes(buffer, length)) {
            throw std::runtime_error("Error generating session token");
        }
        return std::string(reinterpret_cast<const char*>(buffer), length);
    }

    // try {
    //     std::string token = generate_session_token(32);
    //     // do something with the session token
    // } catch (const std::exception& e) {
    //     std::cerr << "Error: " << e.what() << std::endl;
    // }

}

#endif