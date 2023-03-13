#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <openssl/decoder.h>

#include <stdio.h>

#include <utility>
#include <vector>
#include <string>

#include "crypt/base64.hpp"
#include "prizm/prizm.h"
#include "util/file_system.hpp"

typedef unsigned char BYTE;
typedef const char* str_t;
typedef std::vector<BYTE> byte_t;

const size_t KEY_LENGTH = 32;
const size_t IV_LENGTH = 12;
const size_t AUTH_TAG_LENGTH = 16;

static const char *propq = NULL;


namespace jcrypt {

namespace rsa {

std::vector<char*> generate(int bits, std::string session, std::string directory) {
    std::vector<std::string> args{"rsa", std::to_string(bits), session, directory};
    std::vector<char*> c_strings;
    for (const auto& arg : args) {
        c_strings.push_back(const_cast<char*>(arg.c_str()));
    }
    c_strings.push_back(nullptr); 
    return c_strings;
}

int encrypt(const char* in_fname, const char* out_fname, const char* pkey_fname, FILE* error_stream) {
    int exit_code = 0;
    int err = 1;

    FILE* in_file   = NULL;
    FILE* out_file  = NULL;
    FILE* pkey_file = NULL;

    EVP_PKEY* pkey;
    EVP_PKEY_CTX* ctx;

    unsigned char* in_buf;
    unsigned char* out_buf;

    std::string s;

    in_file = fopen(in_fname, "rb");
    if (!in_file) {
        fprintf(stderr, "Could not open input file \"%s\"\n", in_fname);
        goto failure;
    }

    out_file = fopen(out_fname, "wb");
    if (!out_file) {
        fprintf(stderr, "Could not open output file \"%s\"\n", out_fname);
        goto failure;
    }

    pkey_file = fopen(pkey_fname, "rb");
    if (!pkey_file) {
        fprintf(stderr, "Could not open public key file \"%s\"\n", pkey_fname);
        goto failure;
    }

    pkey = NULL;
    ctx = NULL;

    in_buf  = NULL;
    out_buf = NULL;

    ERR_clear_error();

    // BYEL("PUBLIC KEY IS: %s\n", pkey_fname);

    pkey = PEM_read_PUBKEY(pkey_file, NULL, NULL, NULL);
    if (!pkey) {
        if (error_stream)
            fprintf(error_stream, "Could not load public key\n");
        goto failure;
    }

    ctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, NULL);

#define PKEY_CTX_INIT_STYLE 1
#if PKEY_CTX_INIT_STYLE == 1
    err = EVP_PKEY_encrypt_init(ctx);
    err = EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);
#elif PKEY_CTX_INIT_STYLE == 2
    int rsa_padding = RSA_PKCS1_OAEP_PADDING;
    OSSL_PARAM params[] = {
        OSSL_PARAM_construct_int(
            OSSL_ASYM_CIPHER_PARAM_PAD_MODE, &rsa_padding),
        OSSL_PARAM_construct_end()
    };

    err = EVP_PKEY_encrypt_init_ex(ctx, params);
#elif PKEY_CTX_INIT_STYLE == 3
    OSSL_PARAM params[] = {
        OSSL_PARAM_construct_utf8_string(
            OSSL_ASYM_CIPHER_PARAM_PAD_MODE, OSSL_PKEY_RSA_PAD_MODE_OAEP, 0),
        OSSL_PARAM_construct_end()
    };

    err = EVP_PKEY_encrypt_init_ex(ctx, params);
#endif
#undef PKEY_CTX_INIT_STYLE

    size_t pkey_size;
    size_t in_nbytes;
    size_t out_nbytes;

    pkey_size = EVP_PKEY_get_size(pkey);
    in_buf  = (BYTE*)malloc(pkey_size);
    out_buf = (BYTE*)malloc(pkey_size);

    in_nbytes = fread(in_buf, 1, pkey_size, in_file);
    out_nbytes = pkey_size;
    err = EVP_PKEY_encrypt(ctx, out_buf, &out_nbytes, (BYTE*)in_buf, in_nbytes);
    fwrite(out_buf, 1, out_nbytes, out_file);

    // s = base64_encode((const BYTE*)out_buf, out_nbytes);

    // printf("Base64 encoding is:\n%s\n", s.c_str());

    if (ferror(out_file) || ferror(pkey_file)) {
        if (error_stream)
            fprintf(error_stream, "I/O error\n");
        goto failure;
    }

    if (err <= 0) {
        if (error_stream)
            fprintf(error_stream, "EVP_API error\n");
        goto failure;
    }

    goto cleanup;

failure:
    exit_code = 1;
cleanup:
    free(out_buf);
    free(in_buf);
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    if (ERR_peek_error()) {
        exit_code = 1;
        if (error_stream) {
            fprintf(error_stream, "Errors from the OpenSSL error queue:\n");
            ERR_print_errors_fp(error_stream);
        }
    }

    return exit_code;
}

int decrypt(const char* enc_in_fname, const char* unenc_fname, const char* priv_key_fname, FILE* error_stream) {
    int exit_code = 0;
    int err = 1;

    FILE* enc_in_file   = NULL;
    FILE* unenc_file  = NULL;
    FILE* priv_key_file = NULL;

    EVP_PKEY* pkey;
    EVP_PKEY_CTX* ctx;

    unsigned char* in_buf;
    unsigned char* out_buf;

    enc_in_file = fopen(enc_in_fname, "rb");
    if (!enc_in_file) {
        fprintf(stderr, "Could not open encrypted file \"%s\"\n", enc_in_fname);
        goto failure;
    }

    unenc_file = fopen(unenc_fname, "wb");
    if (!unenc_file) {
        fprintf(stderr, "Could not open unencrypted file \"%s\"\n", unenc_fname);
        goto failure;
    }

    priv_key_file = fopen(priv_key_fname, "rb");
    if (!priv_key_file) {
        fprintf(stderr, "Could not open private key file \"%s\"\n", priv_key_fname);
        goto failure;
    }

    pkey = NULL;
    ctx = NULL;

    in_buf  = NULL;
    out_buf = NULL;

    size_t pkey_size;
    size_t in_nbytes;
    size_t out_nbytes;

    ERR_clear_error();

    pkey = PEM_read_PrivateKey(priv_key_file, NULL, NULL, NULL);
    if (!pkey) {
        if (error_stream)
            fprintf(error_stream, "Could not load keypair\n");
        goto failure;
    }

    ctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, NULL);

    err = EVP_PKEY_decrypt_init(ctx);
    err = EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);

    pkey_size = EVP_PKEY_get_size(pkey);
    in_buf  = (BYTE*)malloc(pkey_size);
    out_buf = (BYTE*)malloc(pkey_size);

    in_nbytes = fread(in_buf, 1, pkey_size, enc_in_file);
    out_nbytes = pkey_size;
    err = EVP_PKEY_decrypt(ctx, out_buf, &out_nbytes, in_buf, in_nbytes);

    // printf("Unencrypted: %s\n", (const char*)out_buf);

    fwrite(out_buf, 1, out_nbytes, unenc_file);

    if (ferror(enc_in_file) || ferror(unenc_file) || ferror(priv_key_file)) {
        if (error_stream)
            fprintf(error_stream, "I/O error\n");
        goto failure;
    }

    if (err <= 0) {
        if (error_stream)
            fprintf(error_stream, "EVP_API error\n");
        goto failure;
    }

    goto cleanup;

failure:
    exit_code = 1;
cleanup:
    free(out_buf);
    free(in_buf);
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    if (ERR_peek_error()) {
        exit_code = 1;
        if (error_stream) {
            fprintf(error_stream, "Errors from the OpenSSL error queue:\n");
            ERR_print_errors_fp(error_stream);
        }
    }


    // const char* to_encode = "Well how the fuck are ya?";

    // std::string s = base64_encode((const BYTE*)to_encode, strlen(to_encode));

    // printf("Base64 encoding is:\n%s\n", s.c_str());

    // std::vector<BYTE> decoded = base64_decode(s);

    // std::cout << "Base64 decoded is:" << std::endl;
    // for (auto b : decoded) {
    //     std::cout << b;
    // }
    // std::cout << std::endl;

    return 0;
}

std::vector<BYTE> encrypt_str(const char* to_encrypt, const char* out_fname, const char* pkey_fname, bool is_public, FILE* error_stream) {
    int exit_code = 0;
    int err = 1;
    std::vector<BYTE> result;
    int i;

    FILE* in_file   = NULL;
    FILE* out_file  = NULL;
    FILE* pkey_file = NULL;

    EVP_PKEY* pkey;
    EVP_PKEY_CTX* ctx;

    unsigned char* in_buf;
    unsigned char* out_buf;

    std::string s;

    // in_file = fopen(in_fname, "rb");
    // if (!in_file) {
    //     fprintf(stderr, "Could not open input file \"%s\"\n", in_fname);
    //     goto failure;
    // }

    out_file = fopen(out_fname, "wb");
    if (!out_file) {
        fprintf(stderr, "Could not open output file \"%s\"\n", out_fname);
        goto failure;
    }

    pkey_file = fopen(pkey_fname, "rb");
    if (!pkey_file) {
        fprintf(stderr, "Could not open public key file \"%s\"\n", pkey_fname);
        goto failure;
    }

    pkey = NULL;
    ctx = NULL;

    in_buf  = NULL;
    out_buf = NULL;

    ERR_clear_error();

    // BYEL("PUBLIC KEY IS: %s\n", pkey_fname);

    BYEL("Encrypt key file: %s\n", pkey_fname);

    if (is_public) {

        pkey = PEM_read_PUBKEY(pkey_file, NULL, NULL, NULL);
        if (!pkey) {
            if (error_stream)
                fprintf(error_stream, "crypt::rsa::encrypt_str: Could not load public key!\n");
            goto failure;
        }

    } else {

        pkey = PEM_read_PrivateKey(pkey_file, NULL, NULL, NULL);
        if (!pkey) {
            if (error_stream)
                fprintf(error_stream, "crypt::rsa::encrypt_str: Could not load private key!\n");
            goto failure;
        }

    }

    ctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, NULL);

#define PKEY_CTX_INIT_STYLE 1
#if PKEY_CTX_INIT_STYLE == 1
    err = EVP_PKEY_encrypt_init(ctx);
    err = EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING);
    // err = EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256());
    // err = EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);
#elif PKEY_CTX_INIT_STYLE == 2
    int rsa_padding = RSA_PKCS1_OAEP_PADDING;
    OSSL_PARAM params[] = {
        OSSL_PARAM_construct_int(
            OSSL_ASYM_CIPHER_PARAM_PAD_MODE, &rsa_padding),
        OSSL_PARAM_construct_end()
    };

    err = EVP_PKEY_encrypt_init_ex(ctx, params);
#elif PKEY_CTX_INIT_STYLE == 3
    OSSL_PARAM params[] = {
        OSSL_PARAM_construct_utf8_string(
            OSSL_ASYM_CIPHER_PARAM_PAD_MODE, OSSL_PKEY_RSA_PAD_MODE_OAEP, 0),
        OSSL_PARAM_construct_end()
    };

    err = EVP_PKEY_encrypt_init_ex(ctx, params);
#endif
#undef PKEY_CTX_INIT_STYLE

    size_t pkey_size;
    size_t in_nbytes;
    size_t out_nbytes;

    pkey_size = EVP_PKEY_get_size(pkey);
    in_buf  = (BYTE*)malloc(pkey_size);
    out_buf = (BYTE*)malloc(pkey_size);

    // in_nbytes = fread(in_buf, 1, pkey_size, in_file);
    out_nbytes = pkey_size;
    err = EVP_PKEY_encrypt(ctx, out_buf, &out_nbytes, (BYTE*)to_encrypt, strlen(to_encrypt) + 1);
    fwrite(out_buf, 1, out_nbytes, out_file);

    // s = base64_encode((const BYTE*)out_buf, out_nbytes);

    // printf("Base64 encoding is:\n%s\n", s.c_str());

    if (ferror(out_file) || ferror(pkey_file)) {
        if (error_stream)
            fprintf(error_stream, "I/O error\n");
        goto failure;
    }

    if (err <= 0) {
        if (error_stream)
            fprintf(error_stream, "EVP_API error\n");
        goto failure;
    }

    i = 0;
    while (i < out_nbytes) {
        result.push_back(out_buf[i]);
        i++;
    }

    goto cleanup;


failure:
    exit_code = 1;
cleanup:
    free(out_buf);
    free(in_buf);
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    if (ERR_peek_error()) {
        exit_code = 1;
        if (error_stream) {
            fprintf(error_stream, "Errors from the OpenSSL error queue:\n");
            ERR_print_errors_fp(error_stream);
        }
    }

    // return exit_code;

    return result;
}

std::vector<BYTE> decrypt_str(const unsigned char* to_decrypt, long int in_bytes, const char* unenc_fname, const char* priv_key_fname, bool is_public, FILE* error_stream) {
    int exit_code = 0;
    int err = 1;

    byte_t result;
    int i;

    FILE* pub_key_file = NULL;

    EVP_PKEY* pkey;
    EVP_PKEY_CTX* ctx;

    unsigned char* in_buf;
    unsigned char* out_buf;

    size_t in_nbytes;
    size_t out_nbytes;
    size_t pkey_size;

    pub_key_file = fopen(priv_key_fname, "rb");
    if (!pub_key_file) {
        fprintf(stderr, "Could not open public key file \"%s\"\n", priv_key_fname);
        goto failure;
    }

    if (is_public) {
        pkey = PEM_read_PUBKEY(pub_key_file, NULL, NULL, NULL);
        if (!pkey) {
            if (error_stream)
                fprintf(error_stream, "crypt::rsa::decrypt_str: Could not load public key!\n");
            goto failure;
        }
    } else {
        pkey = PEM_read_PrivateKey(pub_key_file, NULL, NULL, NULL);
        if (!pkey) {
            if (error_stream)
                fprintf(error_stream, "crypt::rsa::decrypt_str: Could not load private key!\n");
            goto failure;
        }
    }

    ctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, NULL);

    err = EVP_PKEY_decrypt_init(ctx);
    err = EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING);
    // err = EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256());

    pkey_size = EVP_PKEY_get_size(pkey);
    in_buf  = (BYTE*)malloc(pkey_size);
    out_buf = (BYTE*)malloc(pkey_size);

    i = 0;
    while (i < in_bytes) {
        in_buf[i] = to_decrypt[i];
        i++;
    }

    in_nbytes = in_bytes;
    out_nbytes = pkey_size;

    err = EVP_PKEY_decrypt(ctx, out_buf, &out_nbytes, in_buf, in_nbytes);

    i = 0;
    while (i < out_nbytes) {
        result.push_back(out_buf[i]);
        i++;
    }

    if (ferror(pub_key_file)) {
        if (error_stream)
            fprintf(error_stream, "I/O error\n");
        goto failure;
    }

    if (err <= 0) {
        if (error_stream)
            fprintf(error_stream, "EVP_API error\n");
        // goto failure;
    }

    goto cleanup;

failure:
    exit_code = 1;
cleanup:
    free(out_buf);
    free(in_buf);
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    if (ERR_peek_error()) {
        exit_code = 1;
        if (error_stream) {
            fprintf(error_stream, "Errors from the OpenSSL error queue:\n");
            ERR_print_errors_fp(error_stream);
        }
    }

    return result;
}

static EVP_PKEY *generate_rsa_key_long(OSSL_LIB_CTX *libctx, unsigned int bits)
{
    EVP_PKEY_CTX *genctx = NULL;
    EVP_PKEY *pkey = NULL;
    unsigned int primes = 2;

    /* Create context using RSA algorithm. "RSA-PSS" could also be used here. */
    genctx = EVP_PKEY_CTX_new_from_name(libctx, "RSA", propq);
    if (genctx == NULL) {
        fprintf(stderr, "EVP_PKEY_CTX_new_from_name() failed\n");
        goto cleanup;
    }

    /* Initialize context for key generation purposes. */
    if (EVP_PKEY_keygen_init(genctx) <= 0) {
        fprintf(stderr, "EVP_PKEY_keygen_init() failed\n");
        goto cleanup;
    }

    /*
     * Here we set the number of bits to use in the RSA key.
     * See comment at top of file for information on appropriate values.
     */
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(genctx, bits) <= 0) {
        fprintf(stderr, "EVP_PKEY_CTX_set_rsa_keygen_bits() failed\n");
        goto cleanup;
    }

    /*
     * It is possible to create an RSA key using more than two primes.
     * Do not do this unless you know why you need this.
     * You ordinarily do not need to specify this, as the default is two.
     *
     * Both of these parameters can also be set via EVP_PKEY_CTX_set_params, but
     * these functions provide a more concise way to do so.
     */
    if (EVP_PKEY_CTX_set_rsa_keygen_primes(genctx, primes) <= 0) {
        fprintf(stderr, "EVP_PKEY_CTX_set_rsa_keygen_primes() failed\n");
        goto cleanup;
    }

    /*
     * Generating an RSA key with a number of bits large enough to be secure for
     * modern applications can take a fairly substantial amount of time (e.g.
     * one second). If you require fast key generation, consider using an EC key
     * instead.
     *
     * If you require progress information during the key generation process,
     * you can set a progress callback using EVP_PKEY_set_cb; see the example in
     * EVP_PKEY_generate(3).
     */
    fprintf(stderr, "Generating RSA key, this may take some time...\n");
    if (EVP_PKEY_generate(genctx, &pkey) <= 0) {
        fprintf(stderr, "EVP_PKEY_generate() failed\n");
        goto cleanup;
    }

    /* pkey is now set to an object representing the generated key pair. */

cleanup:
    EVP_PKEY_CTX_free(genctx);
    return pkey;
}

/*
 * Generates an RSA public-private key pair and returns it.
 * The number of bits is specified by the bits argument.
 *
 * This uses a more concise way of generating an RSA key, which is suitable for
 * simple cases. It is used if -s is passed on the command line, otherwise the
 * long method above is used. The ability to choose between these two methods is
 * shown here only for demonstration; the results are equivalent.
 */
static EVP_PKEY *generate_rsa_key_short(OSSL_LIB_CTX *libctx, unsigned int bits)
{
    EVP_PKEY *pkey = NULL;

    fprintf(stderr, "Generating RSA key, this may take some time...\n");
    pkey = EVP_PKEY_Q_keygen(libctx, propq, "RSA", (size_t)bits);

    if (pkey == NULL)
        fprintf(stderr, "EVP_PKEY_Q_keygen() failed\n");

    return pkey;
}

/*
 * Prints information on an EVP_PKEY object representing an RSA key pair.
 */
static int dump_key(const EVP_PKEY *pkey)
{
    int rv = 0;
    int bits = 0;
    BIGNUM *n = NULL, *e = NULL, *d = NULL, *p = NULL, *q = NULL;

    /*
     * Retrieve value of n. This value is not secret and forms part of the
     * public key.
     *
     * Calling EVP_PKEY_get_bn_param with a NULL BIGNUM pointer causes
     * a new BIGNUM to be allocated, so these must be freed subsequently.
     */
    if (EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_N, &n) == 0) {
        fprintf(stderr, "Failed to retrieve n\n");
        goto cleanup;
    }

    /*
     * Retrieve value of e. This value is not secret and forms part of the
     * public key. It is typically 65537 and need not be changed.
     */
    if (EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_E, &e) == 0) {
        fprintf(stderr, "Failed to retrieve e\n");
        goto cleanup;
    }

    /*
     * Retrieve value of d. This value is secret and forms part of the private
     * key. It must not be published.
     */
    if (EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_D, &d) == 0) {
        fprintf(stderr, "Failed to retrieve d\n");
        goto cleanup;
    }

    /*
     * Retrieve value of the first prime factor, commonly known as p. This value
     * is secret and forms part of the private key. It must not be published.
     */
    if (EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_FACTOR1, &p) == 0) {
        fprintf(stderr, "Failed to retrieve p\n");
        goto cleanup;
    }

    /*
     * Retrieve value of the second prime factor, commonly known as q. This value
     * is secret and forms part of the private key. It must not be published.
     *
     * If you are creating an RSA key with more than two primes for special
     * applications, you can retrieve these primes with
     * OSSL_PKEY_PARAM_RSA_FACTOR3, etc.
     */
    if (EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_FACTOR2, &q) == 0) {
        fprintf(stderr, "Failed to retrieve q\n");
        goto cleanup;
    }

    /*
     * We can also retrieve the key size in bits for informational purposes.
     */
    if (EVP_PKEY_get_int_param(pkey, OSSL_PKEY_PARAM_BITS, &bits) == 0) {
        fprintf(stderr, "Failed to retrieve bits\n");
        goto cleanup;
    }

    /* Output hexadecimal representations of the BIGNUM objects. */
    fprintf(stdout, "\nNumber of bits: %d\n\n", bits);
    fprintf(stderr, "Public values:\n");
    fprintf(stdout, "  n = 0x");
    BN_print_fp(stdout, n);
    fprintf(stdout, "\n");

    fprintf(stdout, "  e = 0x");
    BN_print_fp(stdout, e);
    fprintf(stdout, "\n\n");

    fprintf(stdout, "Private values:\n");
    fprintf(stdout, "  d = 0x");
    BN_print_fp(stdout, d);
    fprintf(stdout, "\n");

    fprintf(stdout, "  p = 0x");
    BN_print_fp(stdout, p);
    fprintf(stdout, "\n");

    fprintf(stdout, "  q = 0x");
    BN_print_fp(stdout, q);
    fprintf(stdout, "\n\n");

    /* Output a PEM encoding of the public key. */
    if (PEM_write_PUBKEY(stdout, pkey) == 0) {
        fprintf(stderr, "Failed to output PEM-encoded public key\n");
        goto cleanup;
    }

    /*
     * Output a PEM encoding of the private key. Please note that this output is
     * not encrypted. You may wish to use the arguments to specify encryption of
     * the key if you are storing it on disk. See PEM_write_PrivateKey(3).
     */
    if (PEM_write_PrivateKey(stdout, pkey, NULL, NULL, 0, NULL, NULL) == 0) {
        fprintf(stderr, "Failed to output PEM-encoded private key\n");
        goto cleanup;
    }

    rv = 1;
cleanup:
    BN_free(n); /* not secret */
    BN_free(e); /* not secret */
    BN_clear_free(d); /* secret - scrub before freeing */
    BN_clear_free(p); /* secret - scrub before freeing */
    BN_clear_free(q); /* secret - scrub before freeing */
    return rv;
}

}

}
