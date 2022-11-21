#include "server/fetch.h"
#include "server/defs.h"
#include "message/message_buffer.h"
#include "util/file_system.hpp"

#include <vector>

#define TIMEOUT2 300.0

void parse_url(char *url, char **hostname, char **port, char** path) {
    printf("URL: %s\n", url);

    char *p;
    p = strstr(url, "://");

    char *protocol = 0;
    if (p) {
        protocol = url;
        *p = 0;
        p += 3;
    } else {
        p = url;
    }

    if (protocol) {
        if (strcmp(protocol, "https")) {
            fprintf(stderr,
                    "Unknown protocol '%s'. Only 'https' is supported.\n",
                    protocol);
            exit(1);
        }
    }

    *hostname = p;
    while (*p && *p != ':' && *p != '/' && *p != '#') ++p;

    *port = "443";
    if (*p == ':') {
        *p++ = 0;
        *port = p;
    }
    while (*p && *p != '/' && *p != '#') ++p;

    *path = p;
    if (*p == '/') {
        *path = p + 1;
    }
    *p = 0;

    while (*p && *p != '#') ++p;
    if (*p == '#') *p = 0;

    printf("hostname: %s\n", *hostname);
    printf("port: %s\n", *port);
    printf("path: %s\n", *path);
}


int send_request(SSL *s, const char *hostname, const char *port, const char *path, std::string type, std::string message) {
    int result = 1;
    char buffer[490000];

    unsigned char char_arr[message.size()]{};
    std::copy(message.cbegin(), message.cend(), char_arr);
    std::vector<unsigned char> v(message.begin(), message.end());
    // BYEL("CHAR VEC SIZE: %li\n", v.size());
    for (const unsigned char& c : v) {
        std::cout << c;
    }

    unsigned char arr[message.size()];
    char carr[message.size() + 1];

    for (int i = 0; i < message.size(); i++) {
        // if (v[i] == '\0') { BRED("NULL TERMINATOR DETECTED\n"); }
        // printf("%u -- %c\n", v[i], (char)v[i]);
        arr[i] = v[i];
        // if (v[i] == '\0') {
        //     carr[i] = 'a';
        // } else {
            carr[i] = (char)v[i];
        // }
    }
    
    // carr[message.size()];
    // free(buf);
    BCYA("%s\n", carr);

    for (int i = 0; i < message.size(); i++) {
        if (v[i] != (unsigned char)carr[i]) { BRED("WRONG\n"); }
    }

    BYEL("CHAR ARRAY SZ.... %li\n", sizeof(arr));
    // BYEL("CHAR ARRAY LEN.... %li\n", strlen(arr));

    BMAG("WHY OH WHY SIZE: %li\n", sizeof(char_arr));
    BMAG("WHY OH WHY LEN: %li\n", strlen((char*)char_arr));
    BCYA("%s\n", message.data());

    // std::cout << message;

    sprintf(buffer, "GET %s HTTP/1.1\r\n", path);
    BYEL("len: %li\n", strlen(buffer));
    sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname, port);
    BYEL("len: %li\n", strlen(buffer));
    sprintf(buffer + strlen(buffer), "Connection: keep-alive\r\n");
    BYEL("len: %li\n", strlen(buffer));
    sprintf(buffer + strlen(buffer), "User-Agent: honpwc https_get 1.0\r\n");
    // sprintf(buffer + strlen(buffer), "Access-Control-Allow-Origin: %s\r\n", "*");
    sprintf(buffer + strlen(buffer), "Jericho: %s\r\n", "true");
    BYEL("len: %li\n", strlen(buffer));
    if (type != "" || type != "undefined") {
        sprintf(buffer + strlen(buffer), "Content-Type: %s\r\n", type.c_str());
        sprintf(buffer + strlen(buffer), "Content-Length: %li\r\n", message.size());
    }
    sprintf(buffer + strlen(buffer), "\r\n");

    memcpy(buffer + strlen(buffer), v.data(), v.size());

    BMAG("CONTENT SIZE: %li\n", message.size());
    BMAG("BUFFER LEN: %li\n", strlen(buffer));
    BMAG("BUFFER SIZE: %li\n", sizeof(buffer));

    if (type == "binary") {
        int a;
        if ((a = SSL_write(s, buffer, sizeof(buffer))) <= 0) {
            printf("Server closed connection\n");
            ERR_print_errors_fp(stderr);
            result = 0;
        }
        BWHI("SENT %i bytes\n", a);
    } else {
        int a;
        if (a = SSL_write(s, buffer, strlen(buffer)) <= 0) {
            printf("Server closed connection\n");
            ERR_print_errors_fp(stderr);
            result = 0;
        }
        BWHI("SENT %i bytes\n", a);
    }
    // printf("Sent Headers:\n%s", buffer);
    return result;
}


SOCKET connect_to_host(const char *hostname, const char *port) {
    // printf("Configuring remote address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if (getaddrinfo(hostname, port, &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }

    // printf("Remote address is: ");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
            address_buffer, sizeof(address_buffer),
            service_buffer, sizeof(service_buffer),
            NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);

    // printf("Creating socket...\n");
    SOCKET server;
    server = socket(peer_address->ai_family,
            peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(server)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        exit(1);
    }

    // int i = 1;
    // if (ioctl(server, FIONBIO, (char*)&i) < 0) {
    //     // PFAIL(ESERVER, "ioctl() failed.");
    // }
    int flags = fcntl(server, F_GETFL, 0);
    // flags &= ~O_NONBLOCK;           // set blocking
    flags |= O_NONBLOCK;            // set non-blocking
    fcntl(server, F_SETFL, flags);

    // connect(sock, (struct sockaddr *)&address, sizeof(address));

    // FD_ZERO(&fdset);
    // FD_SET(sock, &fdset);
    // tv.tv_sec = 10;             /* 10 second timeout */
    // tv.tv_usec = 0;

    // if (select(sock + 1, NULL, &fdset, NULL, &tv) == 1)
    // {
    //     int so_error;
    //     socklen_t len = sizeof so_error;

    //     getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);

    //     if (so_error == 0) {
    //         printf("%s:%d is open\n", addr, port);
    //     }

    int timeout_in_seconds = 20;
    struct timeval tv;
    tv.tv_sec = TIMEOUT2;
    tv.tv_usec = 0;
    setsockopt(server, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));

    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(server, &fdset);
    tv.tv_sec = TIMEOUT2;             /* 10 second timeout */
    tv.tv_usec = 0;

    // BMAG("Thread connecting...\n");
    if (connect(server,
                peer_address->ai_addr, peer_address->ai_addrlen) >= 0) {
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        BMAG("Thread connect to %s:%s failed!\n", hostname, port);
        // pthread_exit(NULL);
    }
    freeaddrinfo(peer_address);


    if (select(server + 1, NULL, &fdset, NULL, &tv) == 1)
    {
        // BMAG("Thread selecting...\n");
        int so_error;
        socklen_t len = sizeof(so_error);

        getsockopt(server, SOL_SOCKET, SO_ERROR, &so_error, &len);

        if (so_error == 0) {
            printf("%s:%s is open\n", hostname, port);
            flags = fcntl(server, F_GETFL, 0);
            flags &= ~O_NONBLOCK;           // set blocking
            // flags |= O_NONBLOCK;            // set non-blocking
            fcntl(server, F_SETFL, flags);
            return server;
        } else {
            printf("socket is closed\n");
            BRED("Thread socket is closed\n");
            return -1;
        }
    } 

    BMAG("Thread connected to %s:%s.\n\n",hostname, port);

    return server;
}

void fetch(Any args) {
    MessageBuffer* buf = (MessageBuffer*)args;
    std::string hostname = buf->hostname;
    std::string port = buf->port;
    std::string fromPort = buf->fromPort;
    std::string dir = buf->dir;
    std::string path = buf->path;
    std::string type = buf->type;
    // MessageQueue* queue = buf->mq;
    std::string message = buf->sent;
    std::string flag = buf->flag;

    BMAG("FECTH SEND SIZE: %li\n", message.size());

    BBLU("BUF PORT: %s\n", buf->port.c_str());
    int iport = std::stoi(buf->port);

    BMAG("FETCHING...\n");

    buf->publish();

    if (flag != "undefined") {
        int t = std::stoi(flag);
        sleep(t);
    }

    // if (port == "8082") {
    // sleep(5);
    // }

    std::string tpath = "./log/" + buf->port + ".node";  
    // BMAG("DIR IS: %s\n", dir.c_str());
    // BMAG("TPATH IS: %s\n", tpath.c_str());
    t_write(iport, tpath.c_str(), "Prepping exchange with ingress");

#if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)) {
        fprintf(stderr, "Failed to initialize.\n");
        // pthread_exit(NULL);
    }
#endif

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    SSL *ssl;
    std::string result = "UNDEFINED";
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    
    if (!ctx) {
        fprintf(stderr, "SSL_CTX_new() failed.\n");
    }

    SOCKET server = connect_to_host(hostname.c_str(), port.c_str());

    int err_status = 0;

    if (server != -1) {

    ssl = SSL_new(ctx);
    if (!ssl) {
        printf("SSL_new() failed.\n");
    }

    if (!SSL_set_tlsext_host_name(ssl, hostname.c_str())) {
        fprintf(stderr, "SSL_set_tlsext_host_name() failed.\n");
        ERR_print_errors_fp(stderr);
    }

    SSL_set_fd(ssl, server);
    if (SSL_connect(ssl) == -1) {
        fprintf(stderr, "SSL_connect() failed.\n");
        ERR_print_errors_fp(stderr);
    }

    // printf ("SSL/TLS using %s\n", SSL_get_cipher(ssl));


    X509 *cert = SSL_get_peer_certificate(ssl);
    if (!cert) {
        fprintf(stderr, "SSL_get_peer_certificate() failed.\n");
        // pthread_exit(NULL);
    }

    char *tmp;
    if ((tmp = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0))) {
        // printf("subject: %s\n", tmp);
        OPENSSL_free(tmp);
    }

    if ((tmp = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0))) {
        // printf("issuer: %s\n", tmp);
        OPENSSL_free(tmp);
    }

    X509_free(cert);

    t_write(iport, tpath.c_str(), "Sending message to node!");

    if (send_request(ssl, hostname.c_str(), fromPort.c_str(), path.c_str(), type, message)) {

     t_write(iport, tpath.c_str(), "Message successfully sent!");

    const clock_t start_time = clock();

    #define RESPONSE_SIZE 32768
    char response[RESPONSE_SIZE+1];
    char *p = response, *q;
    char *end = response + RESPONSE_SIZE;
    char *body = 0;

    enum {length, chunked, connection};
    int encoding = 0;
    int remaining = 0;
    result = "";

    while(1) {

        if ((clock() - start_time) / CLOCKS_PER_SEC > TIMEOUT2) {
            fprintf(stderr, "timeout after %.2f seconds\n", TIMEOUT2);
            // pthread_exit(NULL);
            break;
        } else {
            // printf("%f\n", (clock() - start_time) / CLOCKS_PER_SEC);
        }

        if (p == end) {
            fprintf(stderr, "out of buffer space\n");
            // pthread_exit(NULL);
        }

        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(server, &reads);

        struct timeval timeout;
        timeout.tv_sec = TIMEOUT2;
        timeout.tv_usec = 200;

        if (select(server+1, &reads, 0, 0, &timeout) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            pthread_exit(NULL);
        }

        if (FD_ISSET(server, &reads)) {
            int bytes_received = SSL_read(ssl, p, end - p);
            if (bytes_received < 1) {
                if (encoding == connection && body) {
                    // printf("%.*s", (int)(end - body), body);
                }

                printf("\nConnection closed by peer.\n");
                // pthread_exit(NULL);
                err_status = 1;
                break;
            }

            // printf("Received (%d bytes): '%.*s'\n",
            //         bytes_received, bytes_received, p);

            p += bytes_received;
            *p = 0;

            if (!body && (body = strstr(response, "\r\n\r\n"))) {
                *body = 0;
                body += 4;

                // printf("Received Headers:\n%s\n", response);

                // result += std::string(response) + "\r\n\r\n";

                q = strstr(response, "\nContent-Length: ");
                if (q) {
                    encoding = length;
                    q = strchr(q, ' ');
                    q += 1;
                    remaining = strtol(q, 0, 10);

                } else {
                    q = strstr(response, "\nTransfer-Encoding: chunked");
                    if (q) {
                        encoding = chunked;
                        remaining = 0;
                    } else {
                        encoding = connection;
                    }
                }
                // printf("\nReceived Body:\n");
            }

            if (body) {
                if (encoding == length) {
                    if (p - body >= remaining) {
                        // printf("%.*s", remaining, body);
                        result += std::string(body);
                        break;
                    }
                } else if (encoding == chunked) {
                    do {
                        if (remaining == 0) {
                            if ((q = strstr(body, "\r\n"))) {
                                remaining = strtol(body, 0, 16);
                                if (!remaining) goto finish;
                                body = q + 2;
                            } else {
                                break;
                            }
                        }
                        if (remaining && p - body >= remaining) {
                            // printf("%.*s", remaining, body);
                            result += std::string(body);
                            body += remaining + 2;
                            remaining = 0;
                        }
                    } while (!remaining);
                }
            } //if (body)
        } //if FDSET
    } //end while(1)

    }

    /**
     * TODO: Create a systematic way to switch protocal format: json, json-lines, xml, custom jericho, http, yaml 
     * 
     */
    } else {
        std::string msg = "Failed to connect to " + std::string(hostname) + ":" +
        std::string(port);
        result = "{\"live\": false, \"response\": \""+msg+"\"}";
    }

finish:

    // BMAG("RESPONSE IS: %s\n", result.c_str());
    // t_write(iport, tpath.c_str(), result.c_str());

    if (result == "UNDEFINED") {
        // t_write(iport, tpath.c_str(), "Message from Node is UNDEFINED!");
        // BMAG("Message from Node is UNDEFINED!");
    } else if (result == "") {
        // t_write(iport, tpath.c_str(), "Message from Node is empty. Read failure!");
        // BMAG("Message from Node is empty. Read failure!");
    } else {
        // std::string reply = "Bytes sent to ingress: " + result.size();
        // t_write(iport, tpath.c_str(), reply.c_str());
        // BMAG("%s\n", reply.c_str());
    }

    // buf->mq->publish(result);
    buf->received = result;
    // BGRE("MARKING\n");
    buf->mark();
    // result = "";

    if (server != -1 && err_status != 1) {

        printf("\nClosing socket...\n");
        // SSL_shutdown(ssl);
        // CLOSESOCKET(server);
        // SSL_free(ssl);
        // SSL_CTX_free(ctx);

    #if defined(_WIN32)
        WSACleanup();
    #endif

    }

    BMAG("Finished.\n");
}