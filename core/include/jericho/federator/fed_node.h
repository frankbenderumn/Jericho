#ifndef FEDERATOR_FED_CLIENT_H_
#define FEDERATOR_FED_CLIENT_H_

#include "cluster/cluster_node.h"
#include "util/file_system.hpp"
#include "util/trace.h"
#include "api/api_helper.h"

enum FLStatusType2 {
    FL2_NULL,
    FL2_JOINED,
    FL2_TRAINED,
    FL2_DELIVERED,
    FL2_DORMANT,
    FL2_DROPPED,
    FL2_WAITLISTED,
    FL2_BLACKLISTED,
    FL2_STOPPED,
    FL2_TRAINING,
    FL2_HALTED,
    FL2_EXITED,
    FL2_FINISHED
};

enum FedRole {
    FED_ROLE_NULL,
    FED_ROLE_AGGREGATOR,
    FED_ROLE_CLIENT,
    FED_ROLE_CENTRAL
};

class FedNode : public ClusterNode {
    std::string _parseRole(FedRole role) {
        std::string result = "undefined";
        switch(role) {
            case FED_ROLE_AGGREGATOR:
                result = "aggregator";
                break;
            case FED_ROLE_CLIENT:
                result = "client";
                break;
            case FED_ROLE_CENTRAL:
                result = "main";
                break;
            default:
                break;
        }
        return result;
    }

  protected:
    long long _timestamp;
    long long _updated;
    std::string _dataset = "MNIST";
    FedRole _role = FED_ROLE_NULL;
    FLStatusType2 _status = FL2_NULL; 
    double _lr = 0.001;
    double _targetAccuracy = 95.0;
    double _accuracy = 0.0;
    double _accumLatency = 0.0;
    int _epochs = 5;
    std::string _optimizer = "adam";
    std::string _stop = "undefined";
    std::string _gui = "127.0.0.1:8080";
    std::string _store;
    double _resourceHeterogeneity = 0.0;
    std::vector<std::string> _parents;

  public:
    int served;
    std::string id;
    std::string url;
    std::unordered_map<std::string, std::string> args;
    double _accumTrain = 0.0;
    bool _needsUpdate = false;
    picojson::object _updateParams;
    std::vector<std::vector<std::string>> _queue = {};

    /** should be QuorumMember for better readability */
    // std::string host, std::string port, std::string dir, ClusterIndex* index
    FedNode(std::string host, std::string port, std::string dir, ClusterIndex* index, long long timestamp) {
        PCREATE;
        this->_host = host;
        this->_port = port;
        this->_dir = dir;
        this->_store = "./public/cluster/" + port;
        this->_index = index;
        this->_timestamp = timestamp; 
        this->_updated = timestamp;
        this->_status = FL2_JOINED;
        this->served = 0;
    }

    FedNode() { PCREATE; }

    FLStatusType2 state() { return _status; }

    void state(FLStatusType2 type) { _status = type; }

    const double resource() { return _resourceHeterogeneity; }

    void parents(std::vector<std::string> parents) { _parents = parents; }
    void dropParent(const std::string& parent) {
        for (auto it = _parents.begin(); it != _parents.end(); ++it) {
            if (*it == parent) {
                _parents.erase(it);
                break;
            }
        }
    }


    int parseFederatorLocal(picojson::object fed, std::unordered_map<std::string, std::string>& args) {
        BYEL("Parsing federator...\n");
        BYEL("DONE FEDERATING\n");
        return 0;
    }

    FedNode(std::string id, std::string nodeUrl, std::string role, std::unordered_map<std::string, std::string> args) {
        PCREATE;
        this->id = id;
        // this->url = url;
        if (role == "agg") {
            role = FED_ROLE_AGGREGATOR;
        } else if (role == "cli") {
            role = FED_ROLE_CLIENT;
        } else if (role == "main") {
            role = FED_ROLE_CENTRAL;
        }
        this->args = args;
    }

    void setConfig(std::string host, std::string port) {
        this->_host = host;
        this->_port = port;
    }

    const FedRole role() const { return _role; }

    const std::string gui() const { return _gui; }

    FedNode(std::string nodeUrl, std::string path) {
        PCREATE;
        parseTopology(nodeUrl, path);
    }

    virtual ~FedNode() { PDESTROY; }

    void update() { this->_updated = std::time(NULL); }
    void consistencyCheck() {}
    void parseTopology(std::string nodeUrl, std::string path) {}

    std::string stateStr() { return serializeStatus(); }

    void reset() { 
        _epochs = 5;
        _accuracy = 0.0;
        _accumTrain = 0.0;
        _accumLatency = 0.0;
        _needsUpdate = false;
        _stop = "undefined";
        _queue = {};
        _status = FL2_DORMANT;
    }

    void finish() { _status = FL2_FINISHED; }
    void halt() { _status = FL2_HALTED; }
    void stop() { _status = FL2_STOPPED; }
    void training() { _status = FL2_TRAINING; }
    void train() { _status = FL2_TRAINING; }
    void trained() { _status = FL2_TRAINED; }
    void joined() { _status = FL2_JOINED; }

    std::string serializeStatus() {
        std::string result;
        switch(_status) {
            case FL2_JOINED:
                result = "joined";
                break;
            case FL2_DROPPED:
                result = "dropped";
                break;
            case FL2_DORMANT:
                result = "dormant";
                break;
            case FL2_WAITLISTED:
                result = "waitlisted";
                break;
            case FL2_BLACKLISTED:
                result = "blacklisted";
                break;
            case FL2_TRAINING:
                result = "training";
                break;
            case FL2_STOPPED: result = "stopped"; break;
            case FL2_TRAINED: result = "trained"; break; 
            case FL2_HALTED: result = "halted"; break; 
            case FL2_EXITED: result = "exited"; break; 
            case FL2_FINISHED: result = "finished"; break; 
            default:
                result = "undefined";
                break;
        }
        return result;
    }

    std::string serialize() {
        return "{\"host\": "+_host+", \"port\": "+_port+", \"served\": "+std::to_string(served)+", \"timestamp\": "+std::to_string(_timestamp)+", \"status\": "+serializeStatus()+"}";
    }

    virtual void dump() {}

};

#endif