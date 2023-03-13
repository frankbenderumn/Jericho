#ifndef JERICHO_CRYPT_JWT_H_
#define JERICHO_CRYPT_JWT_H

#include <openssl/hmac.h>

#include "prizm/prizm.h"
#include "crypt/rsa.h"
#include "picojson.h"
#include "system/cgi.h"
#include "util/clock.h"
#include "crypt/crypt.h"

#define JWT_RSA 1
#define JWT_HMAC 0

#ifdef __cplusplus
#define BOOL bool
#else
#define BOOL int
#endif

#define TRUE 1
#define FALSE 0

std::string base64_url_decode(const std::string& input) {
    // Convert URL-safe Base64 to regular Base64
    std::string base64 = input;
    size_t pos = base64.find_first_of("-_");
    while (pos != std::string::npos) {
        base64[pos] = pos == '-' ? '+' : '/';
        pos = base64.find_first_of("-_", pos + 1);
    }

    // Pad the string with '=' characters if needed
    if (base64.length() % 4 != 0) {
        size_t padding = 4 - base64.length() % 4;
        base64.append(padding, '=');
    }

    // Decode the Base64 string
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    BIO* bmem = BIO_new_mem_buf(base64.c_str(), base64.length());
    bmem = BIO_push(b64, bmem);

    std::string output(base64.length(), '\0');
    int len = BIO_read(bmem, &output[0], output.length());

    output.resize(len);
    BIO_free_all(bmem);

    return output;
}

void url_safe(std::string& original) {
    std::replace(original.begin(), original.end(), '+', '-');
    std::replace(original.begin(), original.end(), '/', '_');
    original.erase(std::remove(original.begin(), original.end(), '='), original.end());
}

// Function to print a hexadecimal string
void print_hex(const std::string& label, const unsigned char* data, size_t len) {
    std::cout << label << ": ";
    for (size_t i = 0; i < len; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)data[i];
    }
    std::cout << std::dec << std::endl;
}

std::string hs256_sign(const std::string& message, const std::string& secret) {
    BYEL("Signing with secret: %s (%li)\n", secret.c_str(), secret.size());
    BYEL("Message to sign is:\n%s (%li)\n", message.c_str(), message.size());
    unsigned char signature[EVP_MAX_MD_SIZE];
    unsigned int signature_len = 0;
    HMAC(EVP_sha256(), secret.c_str(), secret.length(),
         reinterpret_cast<const unsigned char*>(message.c_str()), message.size(),
         signature, &signature_len);
    std::string encoded_signature = base64_encode((const BYTE*)signature, signature_len);
    url_safe(encoded_signature);
    return encoded_signature;
}

// bool hs256_verify(const std::string& jwt_token, const std::string& secret) {
//     std::vector<std::string> parts = prizm::tokenize(jwt_token, '.');
//     if (parts.size() != 3) {
//         BRED("Invalid JWT token format!\n");
//         return false;
//     }

//     BYEL("Verify header: %s\n", parts[0]);
//     BYEL("Verify payload: %s\n", parts[1]);
//     BYEL("Verify signature: %s\n", parts[2]);

//     std::string header = base64_url_decode(parts[0]);
//     std::string payload = base64_url_decode(parts[1]);
//     std::string signature = base64_url_decode(parts[2]);
//     std::string expected_signature = hs256_sign(parts[0] + "." + parts[1], secret);
//     return (signature == expected_signature);
// }

// Function to generate a SHA256 signature of a message
std::string sha256_sign(const std::string& message, EVP_PKEY* key) {
    // Create a new message digest context
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    // Initialize the context for SHA256
    const EVP_MD* md = EVP_get_digestbyname("SHA256");
    if (!md) {
        std::cerr << "Error getting message digest" << std::endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }

    if (EVP_DigestSignInit(ctx, nullptr, md, nullptr, key) != 1) {
        std::cerr << "Error initializing signature generation" << std::endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }

    // Update the context with the message data
    if (EVP_DigestSignUpdate(ctx, message.data(), message.length()) != 1) {
        std::cerr << "Error updating signature generation" << std::endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }

    // Determine the size of the signature buffer
    size_t sig_len;
    if (EVP_DigestSignFinal(ctx, nullptr, &sig_len) != 1) {
        std::cerr << "Error determining signature length" << std::endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }

    // Allocate the signature buffer
    std::string signature(sig_len, '\0');

    // Generate the signature
    if (EVP_DigestSignFinal(ctx, (unsigned char*)signature.data(), &sig_len) != 1) {
        std::cerr << "Error generating signature" << std::endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }

    // Clean up the message digest context
    EVP_MD_CTX_free(ctx);

    return signature;
}

// Function to verify a SHA256 signature of a message
bool sha256_verify(const std::string& message, const std::string& signature, EVP_PKEY* key) {
    // Create a new message digest context
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    // Initialize the context for SHA256
    const EVP_MD* md = EVP_get_digestbyname("SHA256");
    if (!md) {
        std::cerr << "Error getting message digest" << std::endl;
        EVP_MD_CTX_free(ctx);
        return false;
    }

    if (EVP_DigestVerifyInit(ctx, nullptr, md, nullptr, key) != 1) {
        std::cerr << "Error initializing signature verification" << std::endl;
        EVP_MD_CTX_free(ctx);
        return false;
    }

    // Update the context with the message data
    if (EVP_DigestVerifyUpdate(ctx, message.data(), message.length()) != 1) {
        std::cerr << "Error updating signature verification" << std::endl;
        EVP_MD_CTX_free(ctx);
        return false;
    }

    // Verify the signature
    int result = EVP_DigestVerifyFinal(ctx, (const unsigned char*)signature.data(), signature.length());

    // Clean up the message digest context
    EVP_MD_CTX_free(ctx);

    // Return true if the signature is valid, false otherwise
    return (result == 1);
}


// bool jwt_verify_signature(const std::string& jwt, const std::string& key_file) {
//     // Split the JWT into its three parts: header, payload, and signature
//     size_t first_dot = jwt.find('.');
//     size_t second_dot = jwt.find('.', first_dot + 1);

//     if (first_dot == std::string::npos || second_dot == std::string::npos) {
//         std::cerr << "Invalid JWT format" << std::endl;
//         return false;
//     }

//     std::string encoded_header = jwt.substr(0, first_dot);
//     std::string encoded_payload = jwt.substr(first_dot + 1, second_dot - first_dot - 1);
//     std::string encoded_signature = jwt.substr(second_dot + 1);

//     // URL-safe Base64 decode the three parts
//     std::string decoded_header = base64_url_decode(encoded_header);
//     std::string decoded_payload = base64_url_decode(encoded_payload);
//     std::string decoded_signature = base64_url_decode(encoded_signature);

//     // Concatenate the decoded header and payload with a period separator
//     std::string decoded_contents = decoded_header + "." + decoded_payload;

//     // Read the public key from a file
//     std::ifstream key_stream(key_file);
//     std::string key((std::istreambuf_iterator<char>(key_stream)), std::istreambuf_iterator<char>());

//     // Verify the signature using the decoded contents and the key
//     EVP_MD_CTX* ctx = EVP_MD_CTX_new();

//     const EVP_MD* md = EVP_get_digestbyname("SHA256");
//     if (!md) {
//         std::cerr << "Error getting message digest" << std::endl;
//         EVP_MD_CTX_free(ctx);
//         return false;
//     }

//     if (EVP_DigestVerifyInit(ctx, nullptr, md, nullptr, EVP_PKEY_new_raw_public_key(EVP_PKEY_RSA, nullptr, (const unsigned char*)key.data(), key.length())) != 1) {
//         std::cerr << "Error initializing signature verification" << std::endl;
//         EVP_MD_CTX_free(ctx);
//         return false;
//     }

//     if (EVP_DigestVerifyUpdate(ctx, decoded_contents.data(), decoded_contents.length()) != 1) {
//         std::cerr << "Error updating signature verification" << std::endl;
//         EVP_MD_CTX_free(ctx);
//         return false;
//     }

//     int result = EVP_DigestVerifyFinal(ctx, (const unsigned char*)decoded_signature.data(), decoded_signature.length());
//     EVP_MD_CTX_free(ctx);

//     if (result == 1) {
//         return true;
//     } else if (result == 0) {
//         std::cerr << "Signature verification failed" << std::endl;
//         return false;
//     } else {
//         std::cerr << "Error performing signature verification" << std::endl;
//         return false;
//     }
// }

namespace jcrypt {

namespace jwt {

    std::string base64_url_encode(const std::string& input) {
        // Create a memory buffer BIO
        BIO* bio = BIO_new(BIO_s_mem());

        // Create a base64 encoding BIO and push it onto the memory buffer BIO
        BIO* b64 = BIO_new(BIO_f_base64());
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        bio = BIO_push(b64, bio);

        // Write the input data to the base64 encoding BIO
        BIO_write(bio, input.data(), input.length());

        // Flush the base64 encoding BIO to get the encoded data as a string
        BIO_flush(bio);
        char* base64_data;
        long base64_length = BIO_get_mem_data(bio, &base64_data);
        std::string output(base64_data, base64_length);

        // Clean up the BIOs
        BIO_free_all(bio);

        std::replace(output.begin(), output.end(), '+', '-');
        std::replace(output.begin(), output.end(), '/', '_');
        output.erase(std::remove(output.begin(), output.end(), '='), output.end());

        return output;
    }

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
        std::string header_encoded = base64_encode((BYTE*)header_serialized.data(), header_serialized.size());
        url_safe(header_encoded);
        // std::string header_encoded = base64_url_encode(header_serialized); 
        std::string payload_encoded = base64_encode((BYTE*)payload.data(), payload.size());
        url_safe(payload_encoded);
        std::string token_encoded = header_encoded + "." + payload_encoded;
        std::string pre_signature = header_serialized + "." + payload;
        std::string signature;
        std::string signature_encoded;
        if (use_rsa) {
            signature = sha256_sign(pre_signature, key);
            signature_encoded = base64_encode((BYTE*)signature.data(), signature.size());            
            url_safe(signature_encoded);
        } else {
            BBLU("Secret: %s\n", keypath);
            signature_encoded = hs256_sign(token_encoded, keypath);
        }
        jwt = token_encoded + "." + signature_encoded;
        return true;
    }

    bool generate_sym(std::string& jwt, picojson::object& payload_obj, std::string& secret, int mins) {
        secret = ::jcrypt::generate_session_token(32);
        // secret = "Good Morning!";
        // not the bug

        // {
        // "expiration": "1678030835",
        // "name": "Jeff",
        // "role": "user"
        // }
        // with secret
        // 7WkndexncK5qqlQdoXwqZS4jycyOt6kcpV9Y24na6l4
        // should equal 
        // wpMD9_kPzVUiAiFKSstbSqB_H8AlJgu9d4CWnS1kLkA
        // according to JWT

        secret = base64_url_encode(secret);
        BMAG("SYM SECRET IS: %s (%li)\n", secret.c_str(), secret.size());
        auto now = Clock::now_chrono();
        now += Min(mins);
        time_t time = Clock::sec(now);
        std::string time_str = std::to_string(time);
        payload_obj["expiration"] = picojson::value(time_str);
        std::string payload = picojson::value(payload_obj).serialize();
        BMAG("SYM PAYLOAD IS: %s (%li)\n", payload.c_str(), payload.size());
        bool result = generate(jwt, payload, secret.c_str(), JWT_HMAC);
        BMAG("GENERATED SYM JWT:\n");
        MAG("%s (%li)\n",jwt.c_str(), jwt.size());
        BLU("Secret: %s (%li)\n", secret.c_str(), secret.size());
        return result;
    }

    BOOL generate_asym(std::string& jwt_token, picojson::object& payload_obj, std::pair<std::string, std::string>& keypair, int mins) {
        std::string id = ::jcrypt::generate_session_token(8);
        id = base64_url_encode(id);
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

        std::string header_decode = base64_url_decode(header_encode);
        std::string payload_decode = base64_url_decode(payload_encode);
        std::string signature_decode = base64_url_decode(signature_encode);

        received_payload = payload_decode;
        std::string signature;

        if (use_rsa) {
            signature = sha256_sign(signature_decode, priv_key);
        } else {
            std::string presign = header_decode + "." + payload_decode;
            BYEL("PRE SIGN: %s (%li)\n", presign.c_str(), presign.size());
            std::string sig_enc = hs256_sign(header_encode + "." + payload_encode, std::string(pub_keypath));

            // std::string sig_enc = base64_url_encode(signature);

            std::string sig_dec_enc = base64_url_encode(signature_decode);

            // std::string sig_enc2 = base64_encode((BYTE*)signature.data(), signature.size());

            // std::string sig_dec_enc2 = base64_encode((BYTE*)signature_decode.data(), signature_decode.size());

            BWHI("======================\n");
            BWHI("1st: %s (%li)\n", sig_enc.c_str(), sig_enc.size());
            BWHI("2nd: %s (%li)\n", sig_dec_enc.c_str(), sig_dec_enc.size());
            BBLU("Tok: %s (%li)\n", signature_encode.c_str(), signature_encode.size());
            // BBLU("1st: %s (%li)\n", sig_enc2.c_str(), sig_enc2.size());
            // BBLU("2nd: %s (%li)\n", sig_dec_enc2.c_str(), sig_dec_enc2.size());
            BWHI("======================\n");

            MAG("Token is:\n%s\n", encoded_token.c_str());
            if (sig_enc == signature_encode) {
                return true;
            } else {
                BRED("THIS IS SO FUCKING GAY!\n");
                return false;
            }
        }


        return sha256_verify(signature_decode, signature, pub_key);
    }

    bool verify_sym(const std::string& jwt_token, picojson::object& extracted_payload, const std::string& secret) {
        std::string received_payload;
        BMAG("VERIFIED SYM JWT:\n");
        MAG("%s (%li)\n",jwt_token.c_str(), jwt_token.size());
        BLU("Secret: %s (%li)\n", secret.c_str(), secret.size());
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