#include "file_system.hpp"
#include "printcolor.hpp"
#include "error.hpp"

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

#ifdef _WIN32
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define SOCKERR() (WSAGetLastError())
#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define SOCKERR() (errno)
#endif

#define FAIL if (!ISVALIDSOCKET(server)) { \
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", SOCKERR()); \
        exit(1); }

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "openssl/crypto.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

void initOpenSSL() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
}

SSL_CTX* getCertificate() {
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        fprintf(stderr, "SSL_CTX_new() failed.\n");
        // PFAIL(ESERVER, "Failed to create SSL ctx");
    }

    if (!SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) || 
        !SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM)) {
        fprintf(stderr, "SSL_CTX_use_certificate_file() failed.\n");
        ERR_print_errors_fp(stderr);
        // PFAIL(ESERVER, "Failed to get SSL ctx");
    }
    return ctx;
}

int receiveRequest(SSL* ssl, char request[]) {
    int bytes = SSL_read(ssl, request, 1024);
    return bytes;
}

int sendRequest(SSL* ssl, char* response) {
    int bytes = SSL_write(ssl, response, strlen(response));
    return bytes;
}