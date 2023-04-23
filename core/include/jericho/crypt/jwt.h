#ifndef JERICHO_CRYPT_JWT_H_
#define JERICHO_CRYPT_JWT_H_

#include <openssl/hmac.h>

#include "prizm/prizm.h"
#include "crypt/rsa.h"
#include "picojson.h"
#include "system/cgi.h"
#include "util/clock.h"
#include "crypt/crypt.h"
#include "crypt/hs256.h"
#include "crypt/sha256.h"

#define JWT_RSA 1
#define JWT_HMAC 0

#ifdef __cplusplus
#ifndef BOOL
#define BOOL bool
#endif
#else
#ifndef BOOL
#define BOOL int
#endif
#endif

#define TRUE 1
#define FALSE 0

// Function to print a hexadecimal string
void print_hex(const std::string& label, const unsigned char* data, size_t len) {
    std::cout << label << ": ";
    for (size_t i = 0; i < len; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)data[i];
    }
    std::cout << std::dec << std::endl;
}

namespace jcrypt {

namespace jwt {

    bool generate(std::string& jwt, std::string payload, const char* keypath, bool use_rsa) {

        EVP_PKEY* key;

        if (use_rsa) {
        
            FILE* key_file = fopen(keypath, "rb");
            if (!key_file) {
                std::cerr << "Error: Failed to open private key file: " << keypath << std::endl;
                return 1;
            }

            key = PEM_read_PrivateKey(key_file, nullptr, nullptr, nullptr);
            if (key == nullptr) {
                std::cerr << "Error: Failed to load private key from file " << keypath << std::endl;
                return false;
            }

        }

        picojson::object header;
        std::string algo;
        (use_rsa) ? algo = "RS256" : algo = "HS256";

        header["alg"] = picojson::value(algo);
        header["typ"] = picojson::value("JWT");
        std::string header_serialized = picojson::value(header).serialize();
        std::string header_encoded = base64::encode_url(header_serialized);
        // std::string header_encoded = base64_url_encode(header_serialized); 
        std::string payload_encoded = base64::encode_url(payload);
        std::string token_encoded = header_encoded + "." + payload_encoded;
        std::string pre_signature = header_serialized + "." + payload;
        std::string signature;
        std::string signature_encoded;
        if (use_rsa) {
            signature = sha256::sign(pre_signature, key);
            signature_encoded = base64::encode_url(signature);            
            // url_safe(signature_encoded);
        } else {
            BBLU("Secret: %s\n", keypath);
            signature_encoded = hs256::sign(token_encoded, keypath);
        }
        jwt = token_encoded + "." + signature_encoded;
        return true;
    }

    bool generate_sym(std::string& jwt, picojson::object& payload_obj, std::string& secret, int mins) {
        BMAG("GENERATING SYM JWT:\n");
        secret = ::jcrypt::generate_session_token(32);
        secret = base64::encode_url(secret);
        auto now = Clock::now_chrono();
        now += Min(mins);
        time_t time = Clock::sec(now);
        std::string time_str = std::to_string(time);
        payload_obj["expiration"] = picojson::value(time_str);
        std::string payload = picojson::value(payload_obj).serialize();
        printf("\t\033[0;35mPayload\033[0m: %s (%li)\n", payload.c_str(), payload.size());
        bool result = generate(jwt, payload, secret.c_str(), JWT_HMAC);
        MAG("\tGenerated token:\n");
        printf("\t%s (%li)\n",jwt.c_str(), jwt.size());
        printf("\t\033[0;35mSecret\033[0m: %s (%li)\n", secret.c_str(), secret.size());
        return result;
    }

    BOOL generate_asym(std::string& jwt_token, picojson::object& payload_obj, std::pair<std::string, std::string>& keypair, int mins) {
        std::string id = ::jcrypt::generate_session_token(8);
        id = base64::encode_url(id);
        std::string dir = "./temp/keys/";
        std::vector<char*> args = ::jcrypt::rsa::generate(4096, id, dir);
        int status = pipe("./bin/rsa", args.data());
        std::string priv_path = dir + id + "__priv.pem";
        std::string pub_path = dir + id + "__pub.pem";        
        keypair = std::pair<std::string, std::string>{pub_path, priv_path};
        auto now = Clock::now_chrono();
        now += Min(mins);
        time_t time = Clock::sec(now);
        std::string time_str = std::to_string(time);
        payload_obj["expiration"] = picojson::value(time_str.c_str());
        std::string payload = picojson::value(payload_obj).serialize();
        BOOL result = generate(jwt_token, payload, priv_path.c_str(), JWT_RSA);
        return result;
    }

    bool verify(std::string encoded_token, std::string& received_payload, const char* pub_keypath, const char* priv_keypath, bool is_public, bool use_rsa) {
        
        EVP_PKEY* key;
        EVP_PKEY* pub_key;
        EVP_PKEY* priv_key;
        
        if (use_rsa) {

            FILE* pub_key_file = fopen(pub_keypath, "rb");
            if (!pub_key_file) {
                std::cerr << "Error: Failed to open key file: " << pub_keypath << std::endl;
                return false;
            }

            FILE* priv_key_file = fopen(priv_keypath, "rb");
            if (!priv_key_file) {
                std::cerr << "Error: Failed to open key file: " << priv_keypath << std::endl;
                return false;
            }
            
            priv_key = PEM_read_PrivateKey(priv_key_file, nullptr, nullptr, nullptr);
            if (priv_key == nullptr) {
                std::cerr << "Error: Failed to load private key from file " << priv_keypath << std::endl;
                return false;
            }

            pub_key = PEM_read_PUBKEY(pub_key_file, nullptr, nullptr, nullptr);
            if (pub_key == nullptr) {
                std::cerr << "Error: Failed to load public key from file " << pub_keypath << std::endl;
                return false;
            }

            if (is_public) { key = pub_key; } else { key = priv_key; }

        }

        std::vector<std::string> tokens = prizm::tokenize(encoded_token, '.');

        if (tokens.size() != 3) {
            BRED("Invalid JWT Encoding\n");
            return false;
        }

        std::string header_encode = tokens[0];
        std::string payload_encode = tokens[1];
        std::string signature_encode = tokens[2];

        std::string header_decode = base64::decode_url(header_encode);
        std::string payload_decode = base64::decode_url(payload_encode);
        std::string signature_decode = base64::decode_url(signature_encode);

        received_payload = payload_decode;
        std::string signature;

        if (use_rsa) {
            signature = sha256::sign(signature_decode, priv_key);
        } else {
            std::string presign = header_decode + "." + payload_decode;
            std::string sig_enc = hs256::sign(header_encode + "." + payload_encode, std::string(pub_keypath));
            std::string sig_dec_enc = base64::encode_url(signature_decode);

            BWHI("======================\n");
            printf("%-16s: %s (%li)\n", "To Verify", signature_encode.c_str(), signature_encode.size());
            printf("%-16s: %s (%li)\n", "Verification", sig_enc.c_str(), sig_enc.size());
            // BWHI("2nd: %s (%li)\n", sig_dec_enc.c_str(), sig_dec_enc.size());
            // BBLU("1st: %s (%li)\n", sig_enc2.c_str(), sig_enc2.size());
            // BBLU("2nd: %s (%li)\n", sig_dec_enc2.c_str(), sig_dec_enc2.size());

            MAG("Token is:\n%s\n\n\n", encoded_token.c_str());
            if (sig_enc == signature_encode) {
                return true;
            } else {
                return false;
            }
        }


        return sha256::verify(signature_decode, signature, pub_key);
    }

    bool verify_sym(const std::string& jwt_token, picojson::object& extracted_payload, const std::string& secret) {
        std::string received_payload;
        BMAG("VERIFYING SYM JWT:\n");
        MAG("\tToken to verify:\n");
        printf("\t%s (%li)\n",jwt_token.c_str(), jwt_token.size());
        printf("\t\033[0;35mVerified with secret\033[0m: %s (%li)\n", secret.c_str(), secret.size());
        if (!verify(jwt_token, received_payload, secret.c_str(), NULL, 0, JWT_HMAC)) {
            BRED("Failed to verify symmetric JWT token!\n");
            return false;
        }

        picojson::value payload_json;
        if (JFS::parseJson(payload_json, received_payload.c_str()) < 0) {
            BRED("Failed to parse JSON payload!\n");
            return false;
        }

        if (!payload_json.is<picojson::object>()) {
            BRED("Payload is not valid JSON!\n");
            return false;
        }

        extracted_payload = payload_json.get<picojson::object>();

        return true;
    }

    BOOL verify_asym(const std::string& jwt_token, picojson::object& extracted_payload, const std::string& session, BOOL is_public = TRUE) {
        std::string pub_path = session + "__pub.pem";
        std::string priv_path = session + "__priv.pem";
        std::string received_payload;
        if(!verify(jwt_token, received_payload, pub_path.c_str(), priv_path.c_str(), is_public, JWT_RSA)) {
            BRED("Failed to verify asymmetric JWT token\n");
            return FALSE;
        }

        picojson::value payload_json;
        if (JFS::parseJson(payload_json, received_payload.c_str()) < 0) {
            BRED("Failed to parse JSON payload!\n");
            return FALSE;
        }

        if (!payload_json.is<picojson::object>()) {
            BRED("Payload is not valid JSON!\n");
            return FALSE;
        }

        extracted_payload = payload_json.get<picojson::object>();

        return TRUE;
    }

}

}

#endif