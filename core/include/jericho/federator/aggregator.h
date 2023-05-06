#ifndef JERICHO_FEDERATOR_AGGREGATOR_H_
#define JERICHO_FEDERATOR_AGGREGATOR_H_

#include "federator/fed_node.h"

class Aggregator : public FedNode {
    std::string _valid = "valid.py";
    std::string _test = "test.py";
    std::string _model = "./py/scripts/torch.pt";
    std::string _agg_weight;
    int _rounds = 3;
    int _round = 0;
    int _quorum = 1;
    int _accumRounds = 0;
    std::string _fusionAlgo = "fed-avg";
    std::string _fusionScript = "fusion.py"; // maybe not needed

    // graph construction
    std::deque<std::string> _clients;
    std::string _parent;
    std::map<std::string, std::string> _weights; 
    std::vector<std::string> _children;
    std::vector<std::string> _blacklist;
    std::deque<std::pair<std::string, std::string>> _readables;
    
    std::string _stopCondition = "ACCURACY";
    std::string _stopValue = "85";

  public:
    bool leader = 0;

    Aggregator(std::string dataset) {
        PCREATE;
        _dataset = dataset;
        _role = FED_ROLE_AGGREGATOR;
    }

    ~Aggregator() { PDESTROY; }

    void readable(std::string client, std::string fpath) {
        _readables.push_back({client, fpath});
    }

    std::pair<std::string, std::string> toRead() {
        std::pair<std::string, std::string> result;
        if (_readables.size() > 0) {
            result = _readables[0];
            _readables.pop_front();
        }
        return result;
    }

    void rounds(int rounds_) {
        _rounds = rounds_;
    }

    bool hasAggregator() { return (_parents.size() != 0); }

    std::vector<std::string> children() const { return _children; }

    std::deque<std::string> clients() const { return _clients; }

    std::vector<std::string> parents() const { return _parents; }

    void parents(std::vector<std::string> parents) { this->_parents = parents;}

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

    const std::string model() const { return _model; }
    void model(std::string model) { _model = model; }

    bool quorumMet() { 
        if (_clients.size() != 0) {
            return (_clients.size() >= _quorum); 
        } else {
            BRED("Aggregator::quorumMet: Quorum not met since _clients is 0\n");
            return 0;
        }
    }

    void storeWeights(const std::string& client, const std::string& weights) {
        _weights[client] = weights;
    }

    int numClients() { 
        return _clients.size();
    }

    bool fuse(std::string& response) {
        if (this->quorumMet()) {
            std::string paths = "[";
            for (auto w : _weights) {
                MAG("weights: %s: %s\n", w.first.c_str(), w.second.c_str());
                paths += w.second + ", ";
            }
            paths.pop_back(); paths.pop_back();
            paths += "]";

            std::string arg0 = "./py/fusion.py";
            std::string arg1 = paths;
            std::string arg2 = std::to_string(_round);
            std::string arg3 = _port;
            std::string arg4 = "./public/cluster/" + _port + "/";
            std::string arg5 = _model;
            // std::string arg5 = mode;
            // std::string arg6 = router->federator()->local()->model();
            std::string command = "python3 " + arg0 + " " + arg1 + " " + arg2 + " " + arg3 + " " + arg4 + " " + arg5;
            std::string results = pipe(command); // need to account for fusing race condition...?
            //  + " " + arg5 + " " + arg6;

            response = results;

            _model = "./public/cluster/" + _port + "/model-" + std::to_string(_round) + ".pt";
            _agg_weight = "./public/cluster/" + _port + "/agg-" + std::to_string(_round) + ".wt";

            MAG("New model: %s\n", _model.c_str());
            MAG("New weights: %s\n", _agg_weight.c_str());
            _round++;

            BMAG("Aggregate::fuse: Model fused successfuly!\n");
            return 1;
        }
        return 0;
    }

    std::string weights() {
        return _agg_weight;
    }

    bool stop() { 
        // if (_round == _rounds) {
        //     _round = 0;
        //     return true;
        // } 
        // return false;
        return (_round == _rounds);
    }

    void dump() override {
        BCYA("Aggregator Dump\n");
        printf("\t\033[1;37m%-16s\033[0m: %s\n", "Leader", (leader) ? "true" : "false");
        printf("\t\033[1;37m%-16s\033[0m: %s\n", "Model", _model.c_str());
        printf("\t\033[1;37m%-16s\033[0m: %s\n", "Fusion", _fusionAlgo.c_str());
        printf("\t\033[1;37m%-16s\033[0m: %s\n", "Stop Condition", _stopCondition.c_str());
        printf("\t\033[1;37m%-16s\033[0m: %s\n", "Stop Value", _stopValue.c_str());
        printf("\t\033[1;37m%-16s\033[0m: %i\n", "Rounds", _rounds);
        printf("\t\033[1;37m%-16s\033[0m: %i\n", "Quorum", _quorum);
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