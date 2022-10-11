#include "cluster/cluster_index.h"
#include "cluster/cluster_node.h"
#include "server/defs.h"

void ClusterIndex::addNode(ClusterNode* node) { _nodes[node->id()] = node; }

void ClusterIndex::addFile(std::string file, int nodeId) { _files[file] = nodeId;}

ClusterNode* ClusterIndex::getCluster(std::string file) {
    if (containsKey(_files, file)) {
        if (containsKey(_nodes, _files[file])) {
            return _nodes[_files[file]];
        } else {
            BRED("Cluster does not exist!");
        }
    } else {
        BRED("Distributed file does not exist!");
    }
    return nullptr;
}

void ClusterIndex::dump() {
    BCYA("INDEX FILES\n");
    for (auto f : _files) {
        BCYA("FILE: %s : %i\n", f.first.c_str(), f.second);
    }
}