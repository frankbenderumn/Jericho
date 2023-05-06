#include "server/fetch.h"
#include "server/defs.h"
#include "util/file_system.hpp"
// #include "crypt/jwt.h"

#include <vector>
#include <string>
#include <unordered_map>

typedef std::unordered_map<std::string, std::string> Args;

#define TIMEOUT2 60.0
#define CHUNK_SIZE 4096

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

size_t num_chunks(size_t size, size_t chunkSize) {
    return size / chunkSize;
}

bool send_request(SSL* ssl, Message* msg) {
    BMAG("Fetch::send_request: Sending Request...\n");
    int result = 1;

    std::string buf;
    if (msg->simple) {
        buf = msg->serialize_simple();
    } else {
        buf = msg->serialize();
    }

    if (buf.size() > 4096) {
        BRED("\tFetch::send_request: serialized message size (%li) is greater than buffer size (%li)\n", buf.size(), (long)4096);
        return 0;
    }

    // BBLU("Fetch::send_request: Buf size: %li\n", buf.size());
    // BBLU("Fetch::send_request: Buf length: %li\n", buf.length());

    char buffer[buf.size()];
    memcpy(buffer, buf.data(), buf.size());

    // MAG("\tFetch::send_request: Message being sent:\n\033[0m%.400s\n", buffer);

    int a = 0;
    if ((a = SSL_write(ssl, buffer, buf.length())) <= 0) {
        printf("Server closed connection\n");
        ERR_print_errors_fp(stderr);
        return 0;
    }

    MAG("Fetch::send_request: Sent %i bytes\n", a);

    return result;
}


SOCKET connect_to_host(const char *hostname, const char *port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if (getaddrinfo(hostname, port, &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        // exit(1);
    }

    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
            address_buffer, sizeof(address_buffer),
            service_buffer, sizeof(service_buffer),
            NI_NUMERICHOST);
    printf("thread-connect_to_host: %s:%s\n", address_buffer, service_buffer);

    SOCKET server;
    server = socket(peer_address->ai_family,
            peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(server)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
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

    // BMAG("Thread connected to %s:%s.\n\n",hostname, port);

    return server;
}

void fetch(Any args) {
    BLU("Fetch: Starting thread!\n");

    Message* buf = static_cast<Message*>(args);
    buf->dump();
    std::string hostname = buf->hostname;
    std::string port = buf->port;
    std::string fromPort = buf->fromPort;
    std::string dir = buf->dir;
    std::string path = buf->path;
    std::string type = buf->type;
    std::string protocol = buf->protocol;
    std::string message = "quirky";
    std::string flag = buf->flag;
    std::unordered_map<std::string, std::string> headers = buf->headers;
    double latency = 0.0;

    SSL *ssl;
    int err_status = 0;
    SOCKET server;
    SSL_CTX* ctx;
    std::string result = "UNDEFINED";
    int iport;
    // std::string tpath = "./log/" + buf->port + ".node";      

    for (char& c : buf->port) {
        if (!std::isdigit(c)) {
            BRED("Fetch: Port %s is not a number\n", port.c_str())
            buf->fulfilled = 2;
            result = "ERROR";
            goto finish;
        }
    }

    iport = std::stoi(buf->port);

    // BLU("Fetch: Variables initialized\n");

    buf->publish();

    // t_write(iport, tpath.c_str(), "Fetch: buffer published");

    if (flag != "undefined") {
        int t = std::stoi(flag);
        // sleep(t);
    }

    // if (port == "8082") {
    // sleep(5);
    // }

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
    ctx = SSL_CTX_new(TLS_client_method());
    
    if (ctx == NULL || ctx == nullptr) {
        BRED("CTX IS NULL SOME FUCKING HOW\n");
    }

    if (!ctx) {
        fprintf(stderr, "SSL_CTX_new() failed.\n");
    }

    server = connect_to_host(buf->toHost.c_str(), buf->toPort.c_str());

    if (server != -1) {

    // BLU("Fetch: Setting Up SSL\n");

    int err = 0;
    ssl = SSL_new(ctx);
    if (!ssl) {
        printf("SSL_new() failed.\n");
        err = 1;
    }

    if (!err) {
        if (!SSL_set_tlsext_host_name(ssl, hostname.c_str())) {
            fprintf(stderr, "SSL_set_tlsext_host_name() failed.\n");
            ERR_print_errors_fp(stderr);
            err = 1;
        }
    }

    if (!err) {
        SSL_set_fd(ssl, server);
        if (SSL_connect(ssl) == -1) {
            fprintf(stderr, "SSL_connect() failed.\n");
            ERR_print_errors_fp(stderr);
            err = 1;
        }
    }

    X509* cert;
    if (!err) {
        cert = SSL_get_peer_certificate(ssl);
        if (!cert) {
            fprintf(stderr, "SSL_get_peer_certificate() failed.\n");
            // pthread_exit(NULL);
            err = 1;
        }
    }

    if (!err) {
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

        Benchmark* bm = bm_start("fetch");

        int chunkCt = 0;
        int chunks = 1;
        size_t chunkSize = 4096;
        if (buf->chunked) chunks = num_chunks(buf->size, chunkSize);
        bool chunked = (chunks > 1);
        std::string chunkStr;

        if (chunks == 1) {
            chunkStr = buf->sent;
        }

        if (send_request(ssl, buf)) {

            // BLU("Fetch: Sending Request Chunk %i/%i!\n", chunkCt+1, chunks);

            const clock_t start_time = clock();

            #define RESPONSE_SIZE 4096
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
                    BRED("Fetch: Timeout after %.2f seconds\n", TIMEOUT2);
                    err_status = 1;
                    break;
                }

                if (p == end) {
                    BRED("Fetch: Out of buffer space\n");
                    err_status = 1;
                    break;
                }

                fd_set reads;
                FD_ZERO(&reads);
                FD_SET(server, &reads);

                struct timeval timeout;
                timeout.tv_sec = TIMEOUT2;
                timeout.tv_usec = 200;

                if (select(server+1, &reads, 0, 0, &timeout) < 0) {
                    BRED("Fetch: select() failed. (%d)\n", GETSOCKETERRNO());
                    break;
                }

                if (FD_ISSET(server, &reads)) {
                    int bytes_received = SSL_read(ssl, p, 4096);
                    if (bytes_received < 1) {
                        // if (encoding == connection && body) {
                        //     // printf("%.*s", (int)(end - body), body);
                        // }

                        BRED("Fetch: No more bytes to receive.\n");
                        break;
                    }

                    printf("Received (%d bytes): '%.*s'\n",
                            bytes_received, bytes_received, p);
                            
                    result = std::string(p, bytes_received);
                } // if FDSET
            } // while(1)
        } // if send_request

        latency = bm_diff(bm);
        CYA("Fetch: Time taken: %.2fms\n", latency * 1000);
        if (latency < TIMEOUT2) {
            buf->latency = latency * 1000;
        }

    } else { // if (!err)
        BRED("Fetch: SSL CONNECT FAILED\n");
    }

    // t_write(iport, tpath.c_str(), "Fetch: buffer published");
    // std::string log = dir + "log/request.log";
    // std::string fileContent = "Request to " + port + " took " + std::to_string(diff) + "ms";
    // JFS::write(log.c_str(), fileContent.c_str());

    /**
     * TODO: Create a systematic way to switch protocal format: json, json-lines, xml, custom jericho, http, yaml 
     * 
     */
    } else { // if (server != -1)
        std::string msg = "Failed to connect to " + std::string(hostname) + ":" +
        std::string(port);
        buf->fulfilled = 2;
        BRED("Fetch: Failed to fetch anything\n");
        result = "{\"live\": false, \"response\": \""+msg+"\"}";
    }

finish:

    // BMAG("RESPONSE IS: %s\n", result.c_str());
    // t_write(iport, tpath.c_str(), result.c_str());

    if (result == "ERROR") {
        buf->fulfilled = 2;
    } else {
        buf->fulfilled = 1;
    }

    // buf->mq->publish(result);

    // BYEL("Fetch: buf->received (%li):\n%s\n", result.size(), result.c_str());

    buf->received = result;

    // std::string end = "Fetch: Marking buffer with results: " + result;
    // BLU("%s\n", end.c_str());

    // CYA("Fetch: Marking buffer with results: %.100s\n", result.c_str());

    // t_write(iport, tpath.c_str(), end.c_str());

    buf->mark();
    // result = "";

    if (server != -1) {

        // t_write(iport, tpath.c_str(), "Closing socket...");
        BMAG("Fetch: Closing socket...\n");
        SSL_shutdown(ssl);
        CLOSESOCKET(server);
        SSL_free(ssl);
        SSL_CTX_free(ctx);

    #if defined(_WIN32)
        WSACleanup();
    #endif

    }

    SEGH

    // t_write(iport, tpath.c_str(), "Fetch: Finished.\n\n");

    // websocket is some fucking how setting modality to 1 ? WTF, need yet another revamp
    // if (buf->modality) {
    //     YEL("Modality is 1 some fucking how!\n");
    //     // may want to check barrier is not null
    //     pthread_barrier_wait(buf->barrier);
    // }

    BYEL("BARRIER FINE!\n");

    BGRE("Fetch: Finished.\n");
}