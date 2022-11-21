#ifndef JERICHO_CLUSTER_CLUSTER_INDEX_H_
#define JERICHO_CLUSTER_CLUSTER_INDEX_H_

#include <unordered_map>
#include <string>

#include "util/iters.h"
#include "cluster/cluster_quorum.h"

class ClusterNode;

class ClusterIndex {

    std::unordered_map<int, ClusterNode*> _nodes;
  
    std::unordered_map<std::string, int> _files;

    std::vector<std::string> _hosts;

    std::unordered_map<std::string, ClusterQuorum*> _quorum;

    int _quorumCt;

  public:
  
    ClusterIndex() {}

    ~ClusterIndex() {
      for (auto q : _quorum) {
        delete q.second;
      }
    }
    
    void addNode(ClusterNode* node);
    
    void addFile(std::string file, int nodeId);
    
    ClusterNode* getCluster(std::string file);

    bool quorumMet(int qct);

    void quorumTrain(const std::vector<ClusterNode*>& neighbors);

    void quorumTrain(std::vector<ClusterQuorum*> neighbors);

    void quorumDrop();

    std::vector<std::string> quorumSelect(int num);

    void quorumUpdate(std::vector<std::string> ports);

    std::string serialize();

    void quorumCt(int q);

    void resetQuorum() {
      for (auto q : _quorum) {
        delete q.second;
      }
      _quorum = {};
      _hosts = {};
    }

    ClusterQuorum* get(std::string host, std::string port) {
      std::string name = host + ":" + port;
      if (prizm::contains_key(_quorum, name)) {
        return _quorum[name];
      }
      return nullptr;
    }

    const std::unordered_map<std::string, ClusterQuorum*>& quorum() const;

    void quorum(std::string host, std::string port, ClusterQuorum* q);

    std::vector<ClusterQuorum*> selectType(FLStatusType type);

    int activateClient(std::string host, std::string port);

    std::vector<std::string> aggs() const;

    const std::vector<std::string>& hosts() const;

    void dump();
};

#endif