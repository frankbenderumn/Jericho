#include "server/peer.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

void cluster_create(Cluster* cluster, Peer* master, const char* cluster_str) {
    cluster->master = master;
    cluster->numSlaves = 0;
    printf("Cluster string is: %s\n", cluster_str);
    char buffer[strlen(cluster_str) + 1];
    strncpy(buffer, cluster_str, strlen(cluster_str) + 1);
    buffer[strlen(cluster_str) + 1] = 0;
    printf("Buffer is: %s\n", buffer);
    char* token;
    char* rest = buffer;
    char master_addr[22];
    printf("Master hostname is: %s\n", master->hostname);
    printf("Master port is: %s\n", master->port);
    strncpy(master_addr, master->hostname, 15);
    strcat(master_addr, ":");
    strncat(master_addr, master->port, 6);
    master_addr[22] = 0;
    printf("Master addr is: %s\n", master_addr);
    char ch = ',';
    int count = 0;
    SEGH
    for(int i = 0; i <= strlen(buffer); i++) {
  		if(buffer[i] == ch) {
  			count++;
 		}
	}
    // 255.255.255.255:65535
    char peer_table[count][21 + 1];
    SEGH
    int i = 0;
    while ((token = strtok_r(rest, ",", &rest))) {
        printf("%s\n", token);
        strncpy(peer_table[i], token, 22); 
        peer_table[i][22] = 0;
        i++;
    }
    for (int j = 0; j < i; j++) {
        printf("table %i: %s\n", j, peer_table[j]);
        char* rest2 = peer_table[j];
        int k = 0;
        if (0 == strcmp(master_addr, peer_table[j])) {
            printf("Master addr detected, skipping...\n");
            continue;
        }
        char* slave_host;
        while ((token = strtok_r(rest2, ":", &rest2))) {
            printf("%s\n", token);
            if (k % 2 == 1) {
                Peer* slave = (Peer*)malloc(sizeof(Peer));
                peer_create(slave, slave_host, token, 10278);
                if (cluster->slaves == NULL) {
                    cluster->slaves = slave;
                } else {
                    cluster->slaves->next = slave;
                }
                cluster->numSlaves++;
            } else {
                slave_host = token;
            }
            k++;
        }
    }
}

PeerState peer_get_state(Peer* peer) {
    PeerState state;
    pthread_mutex_lock(&peer_mutex);
    state = peer->state;
    pthread_mutex_unlock(&peer_mutex);
    return state;
}

void peer_set_state(Peer* peer, PeerState state) {
    pthread_mutex_lock(&peer_mutex);
    peer->state = state;
    pthread_mutex_unlock(&peer_mutex);
}

void peer_create(Peer* peer, const char* hostname, const char* port, int signature) {
    strncpy(peer->hostname, hostname, 16);
    peer->hostname[16] = 0;
    strncpy(peer->port, port, 6);
    peer->port[6] = 0;
    const char* handshake = "JMPP/1.0\r\nConnection: link\r\n\r\nLinking";
    strncpy(peer->sendQueue, handshake, strlen(handshake) + 1);
    peer->sendQueue[strlen(handshake) + 1] = 0;
    peer->sendQueued = strlen(handshake) + 1;
    printf("Peer hostname: %s\n", peer->hostname);
    printf("Peer port: %s\n", peer->port);
    peer->signature = signature;
}

void peer_destroy(Peer* peer) {
    if (peer == NULL) {
        PLOGV(LSERVER, "ERROR", 1, "Attempting to destroy peer that does not exist!");
        exit(1);
    }
    free(peer);
}

void peer(Any arg) {
    Peer* peer = (Peer*)arg;
    // Peer* peer, const char* hostname, const char* port, int signature, const char* request
    // client_set_state(client, SOCKST_PEER);
    DEBUG("OpenSSL version: %s\n", OpenSSL_version(SSLEAY_VERSION));

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "SSL_CTX_new() failed.\n");
        return;
    }

    // if (argc < 3) {
    //     fprintf(stderr, "usage: ./prizm hostname port");
    //     return 1;
    // }

    DEBUG("Configuring remote address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* peer_address;
    if (getaddrinfo(peer->hostname, peer->port, &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", SOCKERR());
        exit(1);
    }

    DEBUG("Remote address is: \n");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, 
        address_buffer, sizeof(address_buffer),
        service_buffer, sizeof(service_buffer),
        NI_NUMERICHOST);
    DEBUG("%s %s\n", address_buffer, service_buffer);

    DEBUG("Creating socket...\n");
    SOCKET server;
    server = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(server)) {
        peer_set_state(peer, PEERST_FAILED);
        DEBUG("socket() failed. (%d)\n", SOCKERR());
        // exit(1);
    }

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    if (setsockopt(server, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
        PERR(ESERVER, "setsockopt() timeout failed.");
    }

    DEBUG("Connecting socket...\n");
    if (connect(server, peer_address->ai_addr, peer_address->ai_addrlen)) {
        peer_set_state(peer, PEERST_FAILED);
        DEBUG("can't connect to peer. (%d)\n", SOCKERR());
        return;
    }

    /** TODO: Send after connect to avoid hang */

    freeaddrinfo(peer_address);
    DEBUG("Connected.\n");

    SSL* ssl = SSL_new(ctx);
    if (!ctx) {
        peer_set_state(peer, PEERST_FAILED);
        DEBUG("SSL_new() failed.\n");
        // fprintf(stderr, "SSL_new() failed.\n");
    }

    if (!SSL_set_tlsext_host_name(ssl, peer->hostname)) {
        peer_set_state(peer, PEERST_FAILED);
        DEBUG("SSL_set_tlsext_host_name() failed.\n");
        // fprintf(stderr, "SSL_set_tlsext_host_name() failed.\n");
        // ERR_print_errors_fp(stderr);
        return;
    }

    SSL_set_fd(ssl, server);
    if (SSL_connect(ssl) == -1) {
        peer_set_state(peer, PEERST_FAILED);
        DEBUG("SSL_connect() failed.\n");
        // ERR_print_errors_fp(stderr);    
        return;    
    }

    DEBUG("SSL/TLS using %s\n", SSL_get_cipher(ssl));

    X509* cert = SSL_get_peer_certificate(ssl);
    if (!cert) {
        DEBUG("SSL_get_peer_certificate() failed.\n");
        peer_set_state(peer, PEERST_FAILED);
        return;
    }

    char* tmp;
    if ((tmp = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0))) {
        PLOGV(LSERVER, "PEER SSL", 6, "subject: %s\n", tmp);
        OPENSSL_free(tmp);
    }

    if ((tmp = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0))) {
        PLOGV(LSERVER, "PEER SSL", 6, "issuer: %s\n", tmp);
        OPENSSL_free(tmp);
    }

    X509_free(cert);

    char buffer[4096];
    // security vulnerabilty? (need snprintf)
    // sprintf(buffer, "GET / HTTP/1.1\r\n");
    // sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname, port);
    // sprintf(buffer + strlen(buffer), "Connection: close\r\n");
    // sprintf(buffer + strlen(buffer), "User-Agent: https_simple\r\n");
    // sprintf(buffer + strlen(buffer), "\r\n");

    SSL_write(ssl, peer->sendQueue, strlen(peer->sendQueue));
    PLOGV(LSERVER, "PEER SEND", 5, "Sent Headers: %i", (int)strlen(peer->sendQueue)); 
    printf("Sent: %s\n", peer->sendQueue);
    // memset(peer->sendQueue, 0, peer->sendQueued);
    // peer->sendQueued = 0;

    while(1) {
        int bytes_received = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytes_received < 1) {
            // BRED("\n Connection closed by peer\n");
            break;
        }
        PLOGV(LSERVER, "PEER RECV", 6, "Received bytes: %i", bytes_received);
        peer_set_state(peer, PEERST_ONLINE);
        printf("Received (%d bytes): '%.*s'\n", bytes_received, bytes_received, buffer);
    }

    // printf("\nClosing socket...\n");
    SSL_shutdown(ssl);
    CLOSESOCKET(server);
    SSL_free(ssl);
    SSL_CTX_free(ctx);

// #ifdef _WIN32
//     WSACleanup();
// #endif

    printf("Finished.\n");
}