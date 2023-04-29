#include "server/fetch.h"
#include "server/defs.h"
#include "util/file_system.hpp"
// #include "crypt/jwt.h"

#include <vector>
#include <string>
#include <unordered_map>

typedef std::unordered_map<std::string, std::string> Args;

#define TIMEOUT2 5.0
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

int send_request(SSL *s, const char *hostname, const char *port, const char *path, Message* buf, std::string chunk, size_t chunkSize, int chunkNum, int chunkTotal) {
    BMAG("Fetch: Sending Request...\n");

    int result = 1;
    char* buffer = (char*)malloc((CHUNK_SIZE));

    std::string message = chunk;
    Args headers = buf->headers;
    std::string type = buf->type;
    std::string protocol = buf->protocol;

    if (strlen(path) > 200) {
        BRED("Fetch::send_request: Path size > 200\n");
        return 0;
    }

    if (strlen(hostname) > 50) {
        BRED("Fetch::send_request: Host size > 50\n");
        return 0;
    }

    if (strlen(port) > 6) {
        BRED("Fetch::send_request: Port size > 6\n");
        return 0;
    }

    // std::cout << message;
    if (protocol == "https") {
        sprintf(buffer, "POST %s HTTP/1.1\r\n", path);
    } else if (protocol == "job") {
        sprintf(buffer, "GET %s JOB\r\n", path);
    }

    sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname, port);
    if (strlen(buffer) > 4000) { BRED("Fetch::send_request: Buffer too large\n"); return 0; }
    sprintf(buffer + strlen(buffer), "Connection: keep-alive\r\n");
    if (strlen(buffer) > 4000) { BRED("Fetch::send_request: Buffer too large\n"); return 0; }
    sprintf(buffer + strlen(buffer), "User-Agent: honpwc https_get 1.0\r\n");
    if (strlen(buffer) > 4000) { BRED("Fetch::send_request: Buffer too large\n"); return 0; }
    // sprintf(buffer + strlen(buffer), "Access-Control-Allow-Origin: %s\r\n", "*");
    sprintf(buffer + strlen(buffer), "Jericho: %s\r\n", "true");
    if (strlen(buffer) > 4000) { BRED("Fetch::send_request: Buffer too large\n"); return 0; }

    if (type == "bin") {
        BGRE("Binary type detected!\n");
        sprintf(buffer + strlen(buffer), "Content-Length: %li\r\n", message.size());
        sprintf(buffer + strlen(buffer), "Content-Type: %s\r\n", "application/octet-stream");        
    } else {
        sprintf(buffer + strlen(buffer), "Content-Length: %li\r\n", message.size());
    }


// range vs. content-range get vs. post
    if (chunkTotal > 1 && type == "bin") {
        sprintf(buffer + strlen(buffer), "Content-Encoding: %s\r\n", "base64");
        sprintf(buffer + strlen(buffer), "Range: %li\r\n", message.size());
    } else if (chunkTotal > 1) {

    } else if (type == "bin") {
        sprintf(buffer + strlen(buffer), "Content-Encoding: %s\r\n", "base64");
    }

    for (auto head : headers) {
        sprintf(buffer + strlen(buffer), "%s: %s\r\n", head.first.c_str(), head.second.c_str());
    }
    sprintf(buffer + strlen(buffer), "\r\n");

    SEGH

    if (type == "bin") {
        std::string encoded = jcrypt::base64::encode_url(message);
        sprintf(buffer + strlen(buffer), encoded.data());
    } else {
        sprintf(buffer + strlen(buffer), message.data());
    }

    // v.clear();

    printf("\tSendRequest: Request content being sent: %.300s\n", buffer);

    if (type == "bin") {
        int a, n = 0, remaining = strlen(buffer);
        while (remaining > 0) {
            int chunk_size = std::min(remaining, CHUNK_SIZE);
            if ((a = SSL_write(s, buffer + n, chunk_size)) <= 0) {
                printf("Server closed connection\n");
                ERR_print_errors_fp(stderr);
                result = 0;
                break;
            }
            printf("SENT %i bytes\n", a);
            remaining -= a;
            n += a;
        }
    } else {
        int a, n = 0, remaining = strlen(buffer);
        while (remaining > 0) {
            int chunk_size = std::min(remaining, CHUNK_SIZE);
            if ((a = SSL_write(s, buffer + n, chunk_size)) <= 0) {
                printf("Server closed connection\n");
                ERR_print_errors_fp(stderr);
                result = 0;
                break;
            }
            printf("FETCH SEND REQUEST - SENT %i bytes\n", a);
            remaining -= a;
            n += a;
        }
    }

    free(buffer);

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
    std::string tpath = "./log/" + buf->port + ".node";      

    for (char& c : buf->port) {
        if (!std::isdigit(c)) {
            BRED("Fetch: Port %s is not a number\n", port.c_str())
            buf->fulfilled = 2;
            result = "ERROR";
            goto finish;
        }
    }

    iport = std::stoi(buf->port);

    BLU("Fetch: Variables initialized\n");

    buf->publish();

    t_write(iport, tpath.c_str(), "Fetch: buffer published");

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

    BLU("Fetch: Setting Up SSL\n");

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

        if (send_request(ssl, hostname.c_str(), fromPort.c_str(), path.c_str(), buf, chunkStr, chunkSize, chunkCt, chunks)) {

        BLU("Fetch: Sending Request Chunk %i/%i!\n", chunkCt+1, chunks);

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
                int bytes_received = SSL_read(ssl, p, end - p);
                if (bytes_received < 1) {
                    if (encoding == connection && body) {
                        // printf("%.*s", (int)(end - body), body);
                    }

                    BRED("Fetch: Connection closed by peer.\n");
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
                } // if (body)
            } // if FDSET
        } // while(1)

        }

        latency = bm_diff(bm);
        CYA("Fetch: Time taken: %.2fms\n", latency * 1000);
        if (latency < TIMEOUT2) {
            buf->latency = latency * 1000;
        }

    } else {
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
    } else {
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
    buf->received = result;

    std::string end = "Fetch: Marking buffer with results: " + result;
    BLU("%s\n", end.c_str());

    // CYA("Fetch: Marking buffer with results: %.100s\n", result.c_str());

    t_write(iport, tpath.c_str(), end.c_str());

    buf->mark();
    // result = "";

    if (server != -1 && err_status != 1) {

        t_write(iport, tpath.c_str(), "Closing socket...");
        CYA("Fetch: Closing socket...\n");
        SSL_shutdown(ssl);
        CLOSESOCKET(server);
        SSL_free(ssl);
        SSL_CTX_free(ctx);

    #if defined(_WIN32)
        WSACleanup();
    #endif

    }

    t_write(iport, tpath.c_str(), "Fetch: Finished.\n\n");

    if (buf->modality) {
        // may want to check barrier is not null
        pthread_barrier_wait(buf->barrier);
    }

    BGRE("Fetch: Finished.\n");
}