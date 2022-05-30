// #ifdef _WIN32
// #ifndef _WIN32_WINNT
// #define _WIN32_WINNT 0x0600
// #endif
// #include <winsock2.h>
// #include <ws2tcpip.h>
// #pragma comment(lib, "ws2_32.lib")
// #else
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>
// #include <unistd.h>
// #include <errno.h>
// #endif

// #ifdef _WIN32
// #define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
// #define CLOSESOCKET(s) closesocket(s)
// #define SOCKERR() (WSAGetLastError())
// #else
// #define ISVALIDSOCKET(s) ((s) >= 0)
// #define CLOSESOCKET(s) close(s)
// #define SOCKET int
// #define SOCKERR() (errno)
// #endif

// #define FAIL if (!ISVALIDSOCKET(server)) { \
//         fprintf(stderr, "getaddrinfo() failed. (%d)\n", SOCKERR()); \
//         exit(1); }

// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <time.h>

// #include <openssl/crypto.h>
// #include <openssl/x509.h>
// #include <openssl/pem.h>
// #include <openssl/ssl.h>
// #include <openssl/err.h>

// int main(int argc, char* argv[]) {
//     printf("OpenSSL version: %s\n", OpenSSL_version(SSLEAY_VERSION));

// #if defined(_WIN32)
//     WSADATA d;
//     if (WSAStartup(MAKEWORD(2, 2), &d)) {
//         fprintf(stderr, "Failed to intialize.\n");
//         return 1;
//     }
// #endif

//     SSL_library_init();
//     OpenSSL_add_all_algorithms();
//     SSL_load_error_strings();

//     SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
//     if (!ctx) {
//         fprintf(stderr, "SSL_CTX_new() failed.\n");
//         return 1;
//     }

//     if (argc < 3) {
//         fprintf(stderr, "usage: ./prizm hostname port");
//         return 1;
//     }

//     char* hostname = argv[1];
//     char* port = argv[2];

//     printf("Configuring remote address...\n");
//     struct addrinfo hints;
//     memset(&hints, 0, sizeof(hints));
//     hints.ai_socktype = SOCK_STREAM;
//     struct addrinfo* peer_address;
//     if (getaddrinfo(hostname, port, &hints, &peer_address)) {
//         fprintf(stderr, "getaddrinfo() failed. (%d)\n", SOCKERR());
//         exit(1);
//     }

//     printf("Remote address is: \n");
//     char address_buffer[100];
//     char service_buffer[100];
//     getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, 
//         address_buffer, sizeof(address_buffer),
//         service_buffer, sizeof(service_buffer),
//         NI_NUMERICHOST);
//     printf("%s %s\n", address_buffer, service_buffer);

//     printf("Creating socket...\n");
//     SOCKET server;
//     server = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
//     if (!ISVALIDSOCKET(server)) {
//         fprintf(stderr, "socket() failed. (%d)\n", SOCKERR());
//         exit(1);
//     }

//     printf("Connecting socket...\n");
//     if (connect(server, peer_address->ai_addr, peer_address->ai_addrlen)) {
//         fprintf(stderr, "connect() failed. (%d)\n", SOCKERR());
//         exit(1);
//     }
//     freeaddrinfo(peer_address);
//     printf("Connected.\n");

//     SSL* ssl = SSL_new(ctx);
//     if (!ctx) {
//         fprintf(stderr, "SSL_new() failed.\n");
//     }

//     if (!SSL_set_tlsext_host_name(ssl, hostname)) {
//         fprintf(stderr, "SSL_set_tlsext_host_name() failed.\n");
//         ERR_print_errors_fp(stderr);
//         return 1;
//     }

//     SSL_set_fd(ssl, server);
//     if (SSL_connect(ssl) == -1) {
//         fprintf(stderr, "SSL_connect() failed.\n");
//         ERR_print_errors_fp(stderr);    
//         return 1;    
//     }

//     printf("SSL/TLS using %s\n", SSL_get_cipher(ssl));

//     X509* cert = SSL_get_peer_certificate(ssl);
//     if (!cert) {
//         fprintf(stderr, "SSL_get_peer_certificate() failed.\n");
//         return 1;
//     }

//     char* tmp;
//     if ((tmp = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0))) {
//         printf("subject: %s\n", tmp);
//         OPENSSL_free(tmp);
//     }

//     if ((tmp = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0))) {
//         printf("issuer: %s\n", tmp);
//         OPENSSL_free(tmp);
//     }

//     X509_free(cert);

//     char buffer[2048];
//     // security vulnerabilty? (need snprintf)
//     sprintf(buffer, "GET / HTTP/1.1\r\n");
//     sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname, port);
//     sprintf(buffer + strlen(buffer), "Connection: close\r\n");
//     sprintf(buffer + strlen(buffer), "User-Agent: https_simple\r\n");
//     sprintf(buffer + strlen(buffer), "\r\n");

//     SSL_write(ssl, buffer, strlen(buffer));
//     printf("Sent Headers:\n%s", buffer);

//     while(1) {
//         int bytes_received = SSL_read(ssl, buffer, sizeof(buffer));
//         if (bytes_received < 1) {
//             printf("\n Connection closed by peer\n");
//             break;
//         }

//         printf("Received (%d bytes): '%.*s'\n", bytes_received, bytes_received, buffer);
//     }

//     printf("\nClosing socket...\n");
//     SSL_shutdown(ssl);
//     CLOSESOCKET(server);
//     SSL_free(ssl);
//     SSL_CTX_free(ctx);

// #ifdef _WIN32
//     WSACleanup();
// #endif

//     printf("Finished.\n");
//     return 0;
// }