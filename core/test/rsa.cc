#include "crypt/rsa.h"
#include "util/file_system.hpp"
#include "prizm/prizm.h"

#include <string>

std::string PUBLIC_DIRECTORY = "./public/frontend";

// key generation
// openssl genrsa -out private.pem 2048

// key extraction
// openssl rsa -in private.pem -outform PEM -pubout -out public.pem

int main(){
 
    // BBLU("START\n");
    // char plainText[2048/8] = "Get some"; //key length : 2048

    // std::string public_path = "./config/keys/public.pem";
    // std::string private_path = "./config/keys/private.pem";

    // // if (!generate_keys(public_path.c_str(), private_path.c_str())) {
    // //     BRED("Failed to generate appropriate RSA keys\n");
    // // }

    // // need to check file existence. Not built into JFS apparently. i.e. .em fails and leads to segfault.
    // // but correct name doesn't segfault
    // std::string public_bytes = JFS::read(public_path.c_str());
    // std::string private_bytes = JFS::read(private_path.c_str());

    // BYEL("PARSED KEYS\n");
    // printf("Private:\n%s\n", private_bytes.c_str());
    // printf("Public:\n%s\n", public_bytes.c_str());
    
    // char publicKey[]="-----BEGIN PUBLIC KEY-----\n"\
    // "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAy8Dbv8prpJ/0kKhlGeJY\n"\
    // "ozo2t60EG8L0561g13R29LvMR5hyvGZlGJpmn65+A4xHXInJYiPuKzrKUnApeLZ+\n"\
    // "vw1HocOAZtWK0z3r26uA8kQYOKX9Qt/DbCdvsF9wF8gRK0ptx9M6R13NvBxvVQAp\n"\
    // "fc9jB9nTzphOgM4JiEYvlV8FLhg9yZovMYd6Wwf3aoXK891VQxTr/kQYoq1Yp+68\n"\
    // "i6T4nNq7NWC+UNVjQHxNQMQMzU6lWCX8zyg3yH88OAQkUXIXKfQ+NkvYQ1cxaMoV\n"\
    // "PpY72+eVthKzpMeyHkBn7ciumk5qgLTEJAfWZpe4f4eFZj/Rc8Y8Jj2IS5kVPjUy\n"\
    // "wQIDAQAB\n"\
    // "-----END PUBLIC KEY-----\n";
    
    // char privateKey[]="-----BEGIN RSA PRIVATE KEY-----\n"\
    // "MIIEowIBAAKCAQEAy8Dbv8prpJ/0kKhlGeJYozo2t60EG8L0561g13R29LvMR5hy\n"\
    // "vGZlGJpmn65+A4xHXInJYiPuKzrKUnApeLZ+vw1HocOAZtWK0z3r26uA8kQYOKX9\n"\
    // "Qt/DbCdvsF9wF8gRK0ptx9M6R13NvBxvVQApfc9jB9nTzphOgM4JiEYvlV8FLhg9\n"\
    // "yZovMYd6Wwf3aoXK891VQxTr/kQYoq1Yp+68i6T4nNq7NWC+UNVjQHxNQMQMzU6l\n"\
    // "WCX8zyg3yH88OAQkUXIXKfQ+NkvYQ1cxaMoVPpY72+eVthKzpMeyHkBn7ciumk5q\n"\
    // "gLTEJAfWZpe4f4eFZj/Rc8Y8Jj2IS5kVPjUywQIDAQABAoIBADhg1u1Mv1hAAlX8\n"\
    // "omz1Gn2f4AAW2aos2cM5UDCNw1SYmj+9SRIkaxjRsE/C4o9sw1oxrg1/z6kajV0e\n"\
    // "N/t008FdlVKHXAIYWF93JMoVvIpMmT8jft6AN/y3NMpivgt2inmmEJZYNioFJKZG\n"\
    // "X+/vKYvsVISZm2fw8NfnKvAQK55yu+GRWBZGOeS9K+LbYvOwcrjKhHz66m4bedKd\n"\
    // "gVAix6NE5iwmjNXktSQlJMCjbtdNXg/xo1/G4kG2p/MO1HLcKfe1N5FgBiXj3Qjl\n"\
    // "vgvjJZkh1as2KTgaPOBqZaP03738VnYg23ISyvfT/teArVGtxrmFP7939EvJFKpF\n"\
    // "1wTxuDkCgYEA7t0DR37zt+dEJy+5vm7zSmN97VenwQJFWMiulkHGa0yU3lLasxxu\n"\
    // "m0oUtndIjenIvSx6t3Y+agK2F3EPbb0AZ5wZ1p1IXs4vktgeQwSSBdqcM8LZFDvZ\n"\
    // "uPboQnJoRdIkd62XnP5ekIEIBAfOp8v2wFpSfE7nNH2u4CpAXNSF9HsCgYEA2l8D\n"\
    // "JrDE5m9Kkn+J4l+AdGfeBL1igPF3DnuPoV67BpgiaAgI4h25UJzXiDKKoa706S0D\n"\
    // "4XB74zOLX11MaGPMIdhlG+SgeQfNoC5lE4ZWXNyESJH1SVgRGT9nBC2vtL6bxCVV\n"\
    // "WBkTeC5D6c/QXcai6yw6OYyNNdp0uznKURe1xvMCgYBVYYcEjWqMuAvyferFGV+5\n"\
    // "nWqr5gM+yJMFM2bEqupD/HHSLoeiMm2O8KIKvwSeRYzNohKTdZ7FwgZYxr8fGMoG\n"\
    // "PxQ1VK9DxCvZL4tRpVaU5Rmknud9hg9DQG6xIbgIDR+f79sb8QjYWmcFGc1SyWOA\n"\
    // "SkjlykZ2yt4xnqi3BfiD9QKBgGqLgRYXmXp1QoVIBRaWUi55nzHg1XbkWZqPXvz1\n"\
    // "I3uMLv1jLjJlHk3euKqTPmC05HoApKwSHeA0/gOBmg404xyAYJTDcCidTg6hlF96\n"\
    // "ZBja3xApZuxqM62F6dV4FQqzFX0WWhWp5n301N33r0qR6FumMKJzmVJ1TA8tmzEF\n"\
    // "yINRAoGBAJqioYs8rK6eXzA8ywYLjqTLu/yQSLBn/4ta36K8DyCoLNlNxSuox+A5\n"\
    // "w6z2vEfRVQDq4Hm4vBzjdi3QfYLNkTiTqLcvgWZ+eX44ogXtdTDO7c+GeMKWz4XX\n"\
    // "uJSUVL5+CVjKLjZEJ6Qc2WZLl94xSwL71E41H4YciVnSCQxVc4Jw\n"\
    // "-----END RSA PRIVATE KEY-----\n";
    
    // std::string encrypt_fail = "Public Encrypt Failed";
    // std::string decrypt_fail = "Public Decrypt Failed";
        
    // unsigned char encrypted[4098]={};
    // unsigned char decrypted[4098]={};
    
    // BBLU("RIP EVERYTHING\n");

    // int encrypted_length = public_encrypt((BYTE*)plainText, strlen(plainText), public_path.c_str(), encrypted);
    // if (encrypted_length == -1) {
    //     BYEL("FUCK!\n");
    //     printLastError((char*)encrypt_fail.c_str());
    //     exit(0);
    // }

    // BGRE("THIS IS PAIN\n");

    // printf("Encrypted length = %d\n", encrypted_length);
    
    // int decrypted_length = private_decrypt(encrypted, encrypted_length, private_path.data(), decrypted);
    // if (decrypted_length == -1) {
    //     printLastError((char*)decrypt_fail.c_str());
    //     exit(0);
    // }

    // printf("Decrypted Text = %s\n", decrypted);
    // printf("Decrypted Length = %d\n", decrypted_length);
    
    // encrypted_length = private_encrypt((BYTE*)plainText, strlen(plainText), private_path.c_str(), encrypted);
    // if( encrypted_length == -1) {
    //     printLastError((char*)encrypt_fail.c_str());
    //     exit(0);
    // }

    // printf("Encrypted length = %d\n", encrypted_length);
    
    // decrypted_length = public_decrypt(encrypted, encrypted_length, public_path.data(), decrypted);
    // if (decrypted_length == -1) {
    //     printLastError((char*)decrypt_fail.c_str());
    //     exit(0);
    // }

    // printf("Decrypted Text = %s\n", decrypted);
    // printf("Decrypted Length = %d\n", decrypted_length);

    // BWHI("WAIT!\n");

    BYEL("Using OpenSSL version \"%s\"\n",
        SSLeay_version (SSLEAY_VERSION));

//     const char* to_encrypt_fname = "./core/test/data/to_encrypt.txt";
//     const char* public_key_fname = "./config/keys/public.pem";
//     const char* private_key_fname = "./config/keys/public.pem";
//     const char* encrypted_fname = "./core/test/data/rsa_encrypt.txt";
//     const char* decrypted_fname = "./core/test/data/rsa_decrypt.txt";

//     FILE* decrypted_file = fopen(decrypted_fname, "wb");
//     if (!decrypted_file) {
//         fprintf(stderr, "Could not open decrypted file \"%s\"\n", decrypted_fname);
//         // goto failure;
//         exit(1);
//     }

//     FILE* encrypted_file = fopen(encrypted_fname, "wb");
//     if (!encrypted_file) {
//         fprintf(stderr, "Could not open encrypted file \"%s\"\n", encrypted_fname);
//         // goto failure;
//         exit(1);
//     }

//     // pkey_file = fopen(pkey_fname, "rb");
//     FILE* public_key_file = fopen(public_key_fname, "rb");
//     if (!public_key_file) {
//         fprintf(stderr, "Could not open public key file \"%s\"\n", public_key_fname);
//         // goto failure;
//         exit(1);
//     }

//     FILE* private_key_file = fopen(private_key_fname, "rb");
//     if (!private_key_file) {
//         fprintf(stderr, "Could not open private key file \"%s\"\n", private_key_fname);
//         // goto failure;
//         exit(1);
//     }

//     FILE* to_encrypt_file = fopen(to_encrypt_fname, "rb");
//     if (!to_encrypt_file) {
//         fprintf(stderr, "Could not open to_encrypt file \"%s\"\n", to_encrypt_fname);
//         // goto failure;
//         exit(1);
//     }

//     {

//     EVP_PKEY* pkey = PEM_read_PUBKEY(public_key_file, NULL, NULL, NULL);

//     EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, NULL);

//     EVP_PKEY_encrypt_init(ctx);

//     EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING);

//     size_t pkey_size = EVP_PKEY_get_size(pkey);

//     unsigned char* in_buf  = (BYTE*)malloc(pkey_size);

//     unsigned char* out_buf = (BYTE*)malloc(pkey_size);

//     size_t in_nbytes = fread(in_buf, 1, pkey_size, to_encrypt_file);

//     size_t out_nbytes = pkey_size;

//     EVP_PKEY_encrypt(ctx, out_buf, &out_nbytes, in_buf, in_nbytes);

//     fwrite(out_buf, 1, out_nbytes, encrypted_file);

//     free(out_buf);

//     free(in_buf);

//     EVP_PKEY_CTX_free(ctx);

//     EVP_PKEY_free(pkey);

//     }

//     {

//     EVP_PKEY* pkey = PEM_read_PrivateKey(private_key_file, NULL, NULL, NULL);

//     EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, NULL);

//     EVP_PKEY_decrypt_init(ctx);
//     EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING);

//     size_t pkey_size = EVP_PKEY_get_size(pkey);

//     BYTE* in_buf = (BYTE*)malloc(pkey_size);

//     BYTE* out_buf = (BYTE*)malloc(pkey_size);

//     size_t in_nbytes = fread(in_buf, 1, pkey_size, encrypted_file);

//     size_t out_nbytes = pkey_size;

//     BBLU("Length of encrypted file is: %li\n", in_nbytes);
//     BBLU("Length of decrypted file is: %li\n", out_nbytes);

//     EVP_PKEY_decrypt(ctx, out_buf, &out_nbytes, in_buf, in_nbytes);

//     fwrite(out_buf, 1, out_nbytes, decrypted_file);

//     free(out_buf);

//     free(in_buf);

//     EVP_PKEY_CTX_free(ctx);

//     EVP_PKEY_free(pkey);

//     }

//     BGRE("MADE IT HERE\n");

//     /* Input data to encrypt */
//     static const unsigned char msg[] =
//         "To be, or not to be, that is the question,\n"
//         "Whether tis nobler in the minde to suffer\n"
//         "The slings and arrowes of outragious fortune,\n"
//         "Or to take Armes again in a sea of troubles";

//     int ret = EXIT_FAILURE;
//     size_t msg_len = sizeof(msg) - 1;
//     size_t encrypted_len = 0, decrypted_len = 0;
//     unsigned char *encrypted = NULL, *decrypted = NULL;
//     OSSL_LIB_CTX *libctx = NULL;

//     SEGH

//     if (!do_encrypt(libctx, msg, msg_len, &encrypted, &encrypted_len)) {
//         BBLU("WHAT THE ACTUAL FUCK!\n");

//         fprintf(stderr, "encryption failed.\n");
//         goto cleanup;
//     }

//     printf("Make me go loco!\n");

//     if (!do_decrypt(libctx, (const char*)encrypted, encrypted_len,
//                     &decrypted, &decrypted_len)) {
//         fprintf(stderr, "decryption failed.\n");
//         goto cleanup;
//     }
//     if (CRYPTO_memcmp(msg, decrypted, decrypted_len) != 0) {
//         fprintf(stderr, "Decrypted data does not match expected value\n");
//         goto cleanup;
//     }
//     ret = EXIT_SUCCESS;

// cleanup:
//     OPENSSL_free(decrypted);
//     OPENSSL_free(encrypted);
//     OSSL_LIB_CTX_free(libctx);
//     if (ret != EXIT_SUCCESS)
//         ERR_print_errors_fp(stderr);


// auto keypair = generate_pair();
// if (keypair.first == NULL) { BRED("1st KEY IS FUCKING NULL!\n"); }
// if (keypair.second == NULL) { BRED("2nd KEY IS FUCKING NULL!\n"); }


// unsigned char str[]=
//     "I am encrypted4332048230948-2308402934702384-2384092384-0234-20384-2384-2384-234";

// unsigned char iv[EVP_MAX_IV_LENGTH]={};

// SEGH

// unsigned char *encrypted_key=(unsigned char*)malloc(EVP_PKEY_size(keypair.first)); 
//                      //https://www.openssl.org/docs/man1.1.1/man3/EVP_SealInit.html


// int encrypted_key_len = EVP_PKEY_size(keypair.first);


// std::vector<unsigned char> cyphered=envelope_seal(&keypair.first,str,strlen((char*)str),
//                                &encrypted_key,&encrypted_key_len,iv);

// std::string cypheredString = get_hex(cyphered);
// printf("%s\n",cypheredString.c_str());

// std::vector<unsigned char> cypheredbinary = get_binary(cypheredString);
// std::vector<unsigned char> plaintext = envelope_open(keypair.second,&cypheredbinary[0],
//                            cypheredbinary.size(),encrypted_key,encrypted_key_len,iv);
// printf("orgin text:%s:End\n",str);
// printf("plain text:");
// for(char c:plaintext)
//     printf("%c",c);

// printf(":End\n");

// free(encrypted_key);
// EVP_PKEY_free(keypair.first);EVP_PKEY_free(keypair.second);
//     return 0;

}