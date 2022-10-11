#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#endif


#if defined(_WIN32)
// #define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
// #define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#else
// #define ISVALIDSOCKET(s) ((s) >= 0)
// #define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <string>

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define TIMEOUT 1.0

void parse_url(char *url, char **hostname, char **port, char** path);

int send_request(SSL *s, char *hostname, char *port, char *path);

SOCKET connect_to_host(char *hostname, char *port, char* path, std::string message);

void fetch(void* arg);