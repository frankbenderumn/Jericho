#ifndef JERICHO_CLUSTER_CLUSTER_H_
#define JERICHO_CLUSTER_CLUSTER_H_

#include <vector>

#include "cluster/cluster_node.h"
#include "cluster/cluster_index.h"
#include "router/router.h"

class Router;

enum ClusterType {
    CLUSTER_NULL,
    CLUSTER_PROXY,
    CLUSTER_DMZ,
    CLUSTER_VPN,
    CLUSTER_FIREWALL,
    CLUSTER_LOAD_BALANCER,
    CLUSTER_FEDERATOR,
    CLUSTER_PARALLEL,
    CLUSTER_RELAY,
    CLUSTER_HONEYPOT,
    CLUSTER_HERRING,
    CLUSTER_MAIN
};

class Cluster {
    ClusterNode* _boss;
    ClusterIndex* _index;
    ClusterType _type;
    
  public:
    Cluster(ClusterType type, ClusterNode* boss, ClusterIndex* index) {
        _type = type;
        _boss = boss;
        _index = index;
    }

    ~Cluster() {
        delete _boss;
        delete _index;
    }

    void pingAll(Router* router, Client* client) {
        BBLU("CLUSTER CHILDREN SIZE: %i\n", (int)_boss->edges()->nodes().size());
        // if ((int)_boss->edges()->nodes().size() > 0) {
        //     _boss->pingOne(router, client, _boss->edges()->nodes()[0]);
        // }
        // for (auto n : _boss->edges()->nodes()) {
        // }
        _boss->pingAll(router, client);
    }

    void pingOne(Router* router, Client* client, ClusterNode* node) {
        BBLU("CLUSTER CHILDREN SIZE: %i\n", (int)_boss->edges()->nodes().size());
        if (node != nullptr) {
            _boss->pingOne(router, client, node);
        }
    }

    void federate(Router* router, Client* client) {
        // for (auto n : _boss->edges()) {
            // n->federate(router, client);
        // }
    }

    void addNode(ClusterNode* node) { _boss->addNode(node); }

    ClusterIndex* index() const { return _index; }

    ClusterNode* boss() const { return _boss; }

    const ClusterType type() const { return _type; }

    void type(ClusterType type) { _type = type; }
};

#endif