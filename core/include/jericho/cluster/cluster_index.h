#ifndef JERICHO_CLUSTER_CLUSTER_INDEX_H_
#define JERICHO_CLUSTER_CLUSTER_INDEX_H_

#include <unordered_map>
#include <string>

#include "util/iters.h"

class ClusterNode;

class ClusterIndex {
    std::unordered_map<int, ClusterNode*> _nodes;
    std::unordered_map<std::string, int> _files;

  public:
    ClusterIndex() {}
    
    void addNode(ClusterNode* node);
    
    void addFile(std::string file, int nodeId);
    
    ClusterNode* getCluster(std::string file);

    void dump();
};

#endif