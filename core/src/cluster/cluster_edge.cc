#include "cluster/cluster_edge.h"
#include "server/defs.h"

ClusterEdge::ClusterEdge(ClusterEdgeType type, std::vector<ClusterNode*> nodes) {
    _type = type;
    _nodes = nodes;
}

ClusterEdge::ClusterEdge(ClusterEdgeType type, ClusterNode* node) {
    _type = type;
    _nodes = std::vector<ClusterNode*>{ node };
}

ClusterEdge::ClusterEdge(std::vector<ClusterEdge*> edges) {
    _type = CLUSTER_EDGE_LAX;
    _edges = edges;
}

ClusterEdge::~ClusterEdge() {
    for (auto edge : _edges) {
        delete edge;
    }
    for (auto node : _nodes) {
        delete node;
    }
}

void ClusterEdge::addEdge(ClusterEdge* edge) {
    if (_type == CLUSTER_EDGE_STRICT) {
        BRED("Can't create hyper edge! Edge is strict!\n");
        return;
    }
    _edges.push_back(edge);
}

void ClusterEdge::addNode(ClusterNode* node) {
    _nodes.push_back(node);
}

const std::vector<ClusterNode*>& ClusterEdge::nodes() {
    if (_type == CLUSTER_EDGE_LAX) {
        BRED("Can't get nodes from hyper edge! Must collapse first!\n");
        return _nodes;
    }

    return _nodes;
}

const std::vector<ClusterNode*>& ClusterEdge::collapse(int idx) {
    if (idx >= _edges.size() || idx < 0) {
        BRED("Invalid idx for node\n");
        return {};
    }

    return _edges[idx]->nodes();
}

ClusterEdgeType ClusterEdge::type() { return _type; }