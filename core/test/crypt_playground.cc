// #include <openssl/core_names.h>
// #include <openssl/err.h>
// #include <openssl/evp.h>
// #include <openssl/params.h>
// #include <openssl/pem.h>
// #include <openssl/rsa.h>
// #include <openssl/decoder.h>

// #include <stdio.h>
// #include <string.h>

// #include "crypt/base64.hpp"
#include "prizm/prizm.h"
// #include "util/file_system.hpp"

// typedef unsigned char BYTE;
// typedef const char* str_t;
// typedef std::vector<BYTE> byte_t;

#include "crypt/rsa.h"

std::string PUBLIC_DIRECTORY = "./public/frontend/";

// std::vector<BYTE> encrypt_str(const char* to_encrypt, const char* out_fname, const char* pkey_fname, FILE* error_stream);

// int decrypt(const char* in_file, const char* out_file, const char* pkey_file, FILE* error_stream);

// std::vector<BYTE> decrypt_str(const unsigned char* to_decrypt, long int in_bytes, const char* out_file, const char* pkey_file, FILE* error_stream);

// static int dump_key(const EVP_PKEY *pkey);

// static EVP_PKEY *generate_rsa_key_short(OSSL_LIB_CTX *libctx, unsigned int bits);

// static EVP_PKEY *generate_rsa_key_long(OSSL_LIB_CTX *libctx, unsigned int bits);

str_t base64_encoded_rsa = "gxwf5DH9XqfHo29RbK4PKmv3rD3SOpQ4v9a3uKVPcl2uYy/GTz"\
"j1GvCDBrqg+cvcTnj/y9Qw3gijuI42WkaC2G+kqYNWioV3Eo3cJ8atuGPuKZI0gb7Sn0bUVrECQl7"\
"MZpC2zpAo87E8twSTYmFgS33xE0msBAnE2bvIVMG85Hqsm5L3SlgTbj0DiaD2SrDlTYNx89keUQKg"\
"/Os9oERb+e+AeTPZaz26vSS69TgMRKttMngaQvmzZIjyA3LB8rtXkLELYbsKrWL/v6hg6F0KboAYt"\
"IG46fQDqh5L1wFsmn+bOgG3ym37Krdh+J4blN+1HZNnLNpSO3Z0Ane5TjbbayPwsFPzbuEJcm6/Ao"\
"uPNtqW6Vu8n+n7cQlZPaq8dVKPcVIOoH+VRkh8sbwW4cC3c0ahk4DFhbKlcJAFPquYFYmvzN6ykwr"\
"ymvAgBK8JFCNngcLZMgdh1bF0tDpK+QBtbj9IJ/FAM2ZQYoAeh3CI/MRCxHsmg+2m/PS9pXuhN6x2"\
"jy7m7hdUfINIPqpcsKucbpBR/6mVsQvjxUFT6L3QyyFtMxgtA+uIVuniUPmIFoEiPGHGylLs/Dalx"\
"ysMcaYp9/KKjx0LFTGBpvH/60jvTnl7fE9jQk7SIaBj8efO7mtaoODVpJC67Bq9eOvyDpXcPYjB8e"\
"E48+McrJ7z4GPC8Qg=";


int main(int argc, char** argv) {
    PrizmInit("RSA Encrypt/Decrypt");

    int exit_code = 0;

    int err;

    const char* in_fname   = "./config/keys/token.txt";
    const char* out_fname  = "./config/keys/public_encrypted.txt";
    const char* pkey_fname = "./config/keys/pub__pokemon.pem";

    const char* enc_in_fname   = "./config/keys/public_encrypted.txt";
    const char* unenc_fname  = "./config/keys/output.txt";
    const char* priv_key_fname = "./config/keys/priv__pokemon.pem";

    const char* to_encrypt = "Roses are red, violets are blue, I hate my life, how bout you?";

    std::vector<BYTE> bytes;
    std::string encoded;
    byte_t decrypted;
    byte_t decoded;

// commented encrypt and decrypt with cli generated rsa keys

    err = jcrypt::rsa::encrypt(in_fname, out_fname, pkey_fname, stderr);
    if (err) {
        fprintf(stderr, "Encryption to file failed\n");
        goto failure;
    }

    // fprintf(stderr, "Encryption to file succeeded\n");

    err = jcrypt::rsa::decrypt(enc_in_fname, unenc_fname, priv_key_fname, stderr);
    if (err) {
        fprintf(stderr, "Decryption to file failed\n");
        goto failure;
    }

    // fprintf(stderr, "Decryption to file succeeded\n");

    TEST(RSAEncrypt, Base64Encoded) {
        bytes = jcrypt::rsa::encrypt_str(to_encrypt, out_fname, pkey_fname, true, stderr);
        encoded = base64_encode((const BYTE*)bytes.data(), bytes.size());
        // ASSERT_STR(encoded.c_str(), base64_encoded_rsa);
    }

    // BBLU("Encoded: %s\n", encoded.c_str());
    decoded = base64_decode(encoded);

    // for (auto b : bytes) {
    //     printf("%c", (char)b);
    // }
    // printf("\n");
    decrypted = jcrypt::rsa::decrypt_str(decoded.data(), decoded.size(), unenc_fname, priv_key_fname, false, stderr);
    TEST(RSAEncrypt, Base64Decoded) {
        ASSERT_STR((str_t)decrypted.data(), to_encrypt);
    }

    // TEST(RSAEncrypt, PrivToPubBase64Encoded) {
    //     bytes = crypt::rsa::encrypt_str(to_encrypt, out_fname, priv_key_fname, false, stderr);
    //     encoded = base64_encode((const BYTE*)bytes.data(), bytes.size());
    //     // ASSERT_STR(encoded.c_str(), base64_encoded_rsa);
    // }

    // BBLU("Encoded: %s\n", encoded.c_str());
    // decoded = base64_decode(encoded);

    // // for (auto b : bytes) {
    // //     printf("%c", (char)b);
    // // }
    // // printf("\n");
    // decrypted = crypt::rsa::decrypt_str(decoded.data(), decoded.size(), unenc_fname, pkey_fname, true, stderr);
    // TEST(RSAEncrypt, PrivToPubBase64Decoded) {
    //     ASSERT_STR((str_t)decrypted.data(), to_encrypt);
    // }

    BMAG("Decrypted string: %s\n", (str_t)decrypted.data());

    goto cleanup;

failure:
    exit_code = 1;

cleanup:
//     if (pkey_file)
//         fclose(pkey_file);
//     if (out_file)
//         fclose(out_file);
//     if (in_file)
//         fclose(in_file);
    // exit(20);

    PrizmResults();
    PrizmCleanup();

    return 0;

// range is 0-255
// return val 256 restarts at 0. May be platform dependent or compiler dependent?
// Should test out on clang with darwin at some point

// 20 - 5120 // 256
// 19 - 4864 // 256
// 104 - 26624 // 256
// 21 - 5376 // 256
// 18 - 4608 // 256
// 11 - 2816
// 1 - 256
////////////////////////////////////////////////////////////////////////////////

}