#ifndef JERICHO_FEDERATOR_AGGREGATOR_H_
#define JERICHO_FEDERATOR_AGGREGATOR_H_

#include "federator/fed_node.h"

class Aggregator : public FedNode {
    std::string _valid = "valid.py";
    std::string _test = "test.py";
    std::string _model = "./py/scripts/torch.pt";
    int _rounds = 3;
    int _round = 0;
    float _quorum = 0.8;
    int _accumRounds = 0;
    std::string _fusionAlgo = "fed-avg";
    std::string _fusionScript = "fusion.py"; // maybe not needed

    // graph construction
    std::deque<std::string> _clients;
    std::map<std::string, std::string> _weights; 
    std::vector<std::string> _children;
    std::vector<std::string> _blacklist;
    
    std::string _stopCondition = "ACCURACY";
    std::string _stopValue = "85";

  public:
    bool leader = 0;

    Aggregator(std::string dataset) {
        PCREATE;
        _dataset = dataset;
    }

    ~Aggregator() { PDESTROY; }

    bool hasAggregator() { return (_parents.size() != 0); }

    void children(std::vector<std::string> children) { _children = children; }

    void addClient(const std::string& client) { _clients.push_back(client); }
    void dropClient(const std::string& client) {
        for (auto it = _clients.begin(); it != _clients.end(); ++it) {
            if (*it == client) {
                _clients.erase(it);
                break;
            }
            _weights.erase(client);
        }
    }

    void dropChild(const std::string& child) {
        for (auto it = _children.begin(); it != _children.end(); ++it) {
            if (*it == child) {
                _children.erase(it);
                break;
            }
        }
    }

    void dropBoth(const std::string& url) { dropParent(url); dropChild(url); }

    const std::string model() const { return JFS::readBinary(_model.c_str()); }
    void model(std::string model) { _model = model; }

    bool quorumMet() { 
        if (_clients.size() != 0) {
            return (_weights.size() / _clients.size() > _quorum); 
        } else {
            BRED("Aggregator::quorumMet: Quorum not met since _clients is 0\n");
            return 0;
        }
    }

    void storeWeights(const std::string& client, const std::string& weights) {
        _weights[client] = weights;
    }

    bool fuse() {
        if (this->quorumMet()) {
            std::string newModel = "ogsdlkjsdkfl\0dflkjasasd\0Sdasdasdasasdasd\0FAKE AGGREGATED MODEL";
            // _model = newModel;
            GRE("new model: %s\n", newModel.c_str());
            BGRE("Aggregate::fuse: Model fused successfuly!\n");
            return 1;
        }
        return 0;
    }

    bool stop() { return true; }

    void dump() override {
        BCYA("Aggregator Dump\n");
        printf("\t\033[1;37m%-16s\033[0m: %s\n", "Leader", (leader) ? "true" : "false");
        printf("\t\033[1;37m%-16s\033[0m: %s\n", "Model", _model.c_str());
        printf("\t\033[1;37m%-16s\033[0m: %s\n", "Fusion", _fusionAlgo.c_str());
        printf("\t\033[1;37m%-16s\033[0m: %s\n", "Stop Condition", _stopCondition.c_str());
        printf("\t\033[1;37m%-16s\033[0m: %s\n", "Stop Value", _stopValue.c_str());
        printf("\t\033[1;37m%-16s\033[0m: %i\n", "Rounds", _rounds);
        printf("\t\033[1;37m%-16s\033[0m: %f\n", "Quorum", _quorum);
        if (_parents.size() == 0) {
            printf("\t\033[1;37m%-16s\033[0m: %s\n", "Parents", "None");
        }
        for (auto& parent : _parents) {
            printf("\t\033[1;37m%-16s\033[0m: %s\n", "Parent", parent.c_str());
        }
        if (_children.size() == 0) {
            printf("\t\033[1;37m%-16s\033[0m: %s\n", "Children", "None");
        }
        for (auto& child : _children) {
            printf("\t\033[1;37m%-16s\033[0m: %s\n", "Child", child.c_str());            
        }
    }


};

#endif