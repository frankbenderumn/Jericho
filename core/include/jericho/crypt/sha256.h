#ifndef CRYPT_SHA256_H_
#define CRYPT_SHA256_H_

#include <string>

namespace jcrypt {

    namespace sha256 {

        // Function to generate a SHA256 signature of a message
        std::string sign(const std::string& message, EVP_PKEY* key) {
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
        bool verify(const std::string& message, const std::string& signature, EVP_PKEY* key) {
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

    }

}

#endif