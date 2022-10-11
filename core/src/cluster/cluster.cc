// #include "cluster/cluster.h"

// Cluster::Cluster(ClusterType type, ClusterNode* boss, ClusterIndex* index) {
//     _type = type;
//     _boss = boss;
//     _index = index;
// }

// Cluster::~Cluster() {
//     delete _boss;
//     delete _index;
// }

// void Cluster::ping(Router* router, Client* client) {
//     for (auto n : _boss->edge()->nodes()) {
//         n->ping(router, client);
//     }
// }

// void Cluster::federate(Router* router, Client* client) {
//     for (auto n : _boss->edge()->nodes()) {
//         n->federate(router, client);
//     }
// }

// void Cluster::addNode(ClusterNode* node) { _boss->addNode(node); }

// ClusterIndex* Cluster::index() const { return _index; }

// ClusterNode* Cluster::boss() const { return _boss; }

// const ClusterType Cluster::type() const { return _type; }

// void Cluster::type(ClusterType type) { _type = type; }