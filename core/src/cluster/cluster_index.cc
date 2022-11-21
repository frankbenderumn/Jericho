#include "cluster/cluster_index.h"
#include "cluster/cluster_node.h"
#include "server/defs.h"

void ClusterIndex::addNode(ClusterNode* node) { 
    _nodes[node->id()] = node; 
    std::string name = node->host() + ":" + node->port();
    _hosts.push_back(name);
}

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

void ClusterIndex::quorumTrain(const std::vector<ClusterNode*>& neighbors) {
    for (auto n : neighbors) {
        std::string name = n->host() + ":" + n->port();
        if (prizm::contains_key(_quorum, name)) {
            FLStatusType t = _quorum[name]->status;
            if (t == FL_JOINED) {
                t = FL_TRAINING;
            }
        }
    }
}

void ClusterIndex::quorumTrain(std::vector<ClusterQuorum*> neighbors) {
    for (auto q : neighbors) {
        if (q->status == FL_JOINED) {
            q->status = FL_TRAINING;
        }
    }
}

void ClusterIndex::quorumDrop() {
    for (auto q : _quorum) {
        if (q.second->status != FL_DELIVERED) {
            if (q.second->status == FL_TRAINING) {
                q.second->status = FL_DROPPED;
            }
        }
    }
}

struct QuorumSorter {
    inline bool operator() (ClusterQuorum* struct1, ClusterQuorum* struct2)
    {
        return (struct1->rounds < struct2->rounds);
    }
};

std::vector<std::string> ClusterIndex::quorumSelect(int num) {
    std::vector<ClusterQuorum*> vec;
    std::vector<std::string> ports;
    for (auto q : _quorum) {
        vec.push_back(q.second);
    }
    std::sort(vec.begin(), vec.end(), QuorumSorter());
    int i = 0;
    if (num > vec.size()) { 
        BRED("SELECTING MORE THAN IN QUORUM! THIS SHOULD NEVER HAPPEN: VEC SIZE IS: %li\n", vec.size());
    } else {
        while (i < num) {
            ports.push_back(vec[i]->port);
            i++;
        }
    }
    return ports;
}

std::vector<ClusterQuorum*> ClusterIndex::selectType(FLStatusType type) {
    std::vector<ClusterQuorum*> vec;
    for (auto q : _quorum) {
        vec.push_back(q.second);
    }
    std::sort(vec.begin(), vec.end(), QuorumSorter());
    std::vector<ClusterQuorum*> result;
    for (auto q : vec) {
        if (q->status == type) {
            result.push_back(q);
        }
    }
    return result;
}

void ClusterIndex::quorumUpdate(std::vector<std::string> ports) {
    for (auto p : ports) {
        std::string name = "127.0.0.1:" + p; /** need to change from local environment */
        _quorum[name]->rounds++;
        _quorum[name]->status = FL_DORMANT;
    }
}

bool ClusterIndex::quorumMet(int qct) {
    int ct = 0;
    for (auto q : _quorum) {
        if (q.second->status == FL_DELIVERED) {
            ct += 1;
        }
    }
    if (ct >= qct) {
        return true;
    }
    return false;
}

void ClusterIndex::quorum(std::string host, std::string port, ClusterQuorum* q) {
    std::string hostname = host + ":" + port;
    _quorum[hostname] = q;
}

const std::unordered_map<std::string, ClusterQuorum*>& ClusterIndex::quorum() const {
    return _quorum;
}

int ClusterIndex::activateClient(std::string host, std::string port) {
    std::string hostname = host + ":" + port;
    for (auto q : _quorum) {
        // BWHI("CLIENT NAME: %s == \n", q.first.c_str());
        if (q.first == hostname) {
            q.second->status = FL_DELIVERED;
            return 0;
        }
    }
    return -1;
}

std::string ClusterIndex::serialize() {
    std::string result = "{";
    for (auto q : _quorum) {
        result + "\""+q.second->port+"\": " + q.second->serialize() + ", "; 
    }
    result.pop_back(); result.pop_back();
    result += "}";
    return result;
}

std::vector<std::string> ClusterIndex::aggs() const {
    std::vector<std::string> hosts;
    for (auto q : _quorum) {
        if (q.second->status != 0) {
            hosts.push_back(q.first);
        }
    }
    return hosts;
}

void ClusterIndex::quorumCt(int q) {
    _quorumCt = q;
}

const std::vector<std::string>& ClusterIndex::hosts() const {
    return _hosts;
}