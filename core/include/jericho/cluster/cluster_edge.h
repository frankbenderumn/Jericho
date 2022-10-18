#ifndef JERICHO_CLUSTER_CLUSTER_EDGE_H_
#define JERICHO_CLUSTER_CLUSTER_EDGE_H_

#include <vector>

class ClusterNode;

enum ClusterEdgeType {
    CLUSTER_EDGE_LAX,
    CLUSTER_EDGE_STRICT
};

class ClusterEdge {
    
    std::vector<ClusterEdge*> _edges;

    std::vector<ClusterNode*> _nodes;
    
    ClusterEdgeType _type = CLUSTER_EDGE_STRICT;
    
  public:
    
    ClusterEdge(ClusterEdgeType type, std::vector<ClusterNode*> nodes);

    ClusterEdge(ClusterEdgeType type, ClusterNode* node);

    ClusterEdge(std::vector<ClusterEdge*> edges);

    ~ClusterEdge();

    void addEdge(ClusterEdge* edge);

    void addNode(ClusterNode* node);

    const std::vector<ClusterNode*>& nodes();

    const std::vector<ClusterNode*>& collapse(int idx);

    ClusterEdgeType type();
};

#endif