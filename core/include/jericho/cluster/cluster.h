#ifndef JERICHO_CLUSTER_CLUSTER_H_
#define JERICHO_CLUSTER_CLUSTER_H_

#include <vector>

#include "cluster/cluster_node.h"
#include "cluster/cluster_index.h"
#include "system/system.h"

class System;

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

    void pingAll(System* router, std::string url) {
        BBLU("CLUSTER CHILDREN SIZE: %i\n", (int)_boss->edges()->nodes().size());
        _boss->pingAll(router, url);
    }

    void pingOne(System* router, std::string url, ClusterNode* node) {
        BBLU("CLUSTER CHILDREN SIZE: %i\n", (int)_boss->edges()->nodes().size());
        if (node != nullptr) {
            _boss->pingOne(router, url, node);
        }
    }

    void pingSet(System* router, std::string url, std::vector<std::pair<std::string, std::string>> set) {
        BBLU("CLUSTER SET SIZE: %i\n", (int)set.size());
        _boss->pingAll(router, url, set);
    }

    void federate(System* router, std::string url) {
        // for (auto n : _boss->edges()) {
            // n->federate(router, client);
        // }
    }

    bool join(std::string host, std::string port) {
        std::string dir = "./public/cluster/" + port;
        if (!contains(_index->hosts(), host + ":" + port)) {
            ClusterNode* node = new ClusterNode(host, port, dir, _index);
            ClusterQuorum* quor = new ClusterQuorum(host, port, node->timestamp()); 
            _index->quorum(host, port, quor);
            BYEL("SIZE OF QUORUM IS NOW: %li\n", _index->quorum().size());
            BYEL("JOINING client to cluster...\n");
            if (_boss == nullptr) {
                BRED("BOSS IS NULL! SHOULD NOT BE POSSIBLE!\n");
            }
            _boss->addNode(node);
            BBLU("JOINED client to cluster\n");
            for (auto child : _boss->nodes()) {
                child->print();
            }
            if (node != nullptr) return true;
        }
        return false;
        BGRE("COMPLETED JOIN\n");
    }

    void addNode(ClusterNode* node) { _boss->addNode(node); }

    ClusterIndex* index() const { return _index; }

    ClusterNode* boss() const { return _boss; }

    const ClusterType type() const { return _type; }

    void type(ClusterType type) { _type = type; }

    MessageBuffer* buffer(std::string url, std::string path) { return _boss->buffer(url, path); }
};

#endif