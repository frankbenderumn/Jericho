#ifndef SERVER_PEER_H_
#define SERVER_PEER_H_ 

#include "server/defs.h"
#include "server/client.h"

static pthread_mutex_t peer_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef enum PeerState {
    PEERST_NULL,
    PEERST_OFFLINE,
    PEERST_MASTER,
    PEERST_CONNECTING,
    PEERST_ONLINE,
    PEERST_CLOSING,
    PEERST_CLOSED,
    PEERST_BROADCASTING,
    PEERST_CACHING,
    PEERST_FAILED
} PeerState;

typedef struct Peer {
    // SSL* ssl;
    char hostname[16];
    char port[8];
    int signature;
    PeerState state;
    struct Peer* next;
    int sendQueued;
    int recvQueued;
    char sendQueue[4096];
    char recvQueue[4096];
} Peer;

typedef struct Cluster {
    Peer* master;
    Peer* slaves;
    int numSlaves;
} Cluster;

void cluster_create(Cluster* cluster, Peer* master, const char* cluster_str);

void peer_create(Peer* peer, const char* hostname, const char* port, int signature);

void peer_set_state(Peer* peer, PeerState state);

PeerState peer_get_state(Peer* peer);

void peer(Any arg);

#endif