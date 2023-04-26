#ifndef FEDERATOR_FED_CLIENT_H_
#define FEDERATOR_FED_CLIENT_H_

#include "cluster/cluster_node.h"
#include "util/file_system.hpp"
#include "util/trace.h"

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
    
    // std::vector<std::string> _fedNeighbors;
    // std::vector<std::string> _clients;
    // std::unordered_map<std::string, int> _pending = {};

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

    void finish() { _status = FL2_FINISHED; }

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
        // for aggregator
        // if (fed.find("fusion") != fed.end()) { args["fusion"] = fed["fusion"].get<std::string>(); this->_fusion = args["fusion"];}
        // if (fed.find("train") != fed.end()) { args["train"] = fed["train"].get<std::string>(); this->_train = args["train"];}
        // if (fed.find("test") != fed.end()) { args["test"] = fed["test"].get<std::string>(); this->_test = args["test"];}
        // if (fed.find("quorum") != fed.end()) { args["quorum"] = std::to_string((int)fed["quorum"].get<double>()); this->_quorum = std::stoi(args["quorum"]);}
        // if (fed.find("rounds") != fed.end()) { this->_rounds = (int)fed["rounds"].get<double>(); }

        // for client
        // if (fed.find("resource") != fed.end()) { this->_resource = fed["resource"].get<double>(); }


        BYEL("Parsing federator...\n");
        // if (fed.find("lr") != fed.end()) { args["lr"] = std::to_string(fed["lr"].get<double>()); this->_lr = std::stod(args["lr"]); }
        // if (fed.find("optimizer") != fed.end()) { args["optimizer"] = fed["optimizer"].get<std::string>(); this->_optimizer = args["optimizer"]; }
        // if (fed.find("algo") != fed.end()) { args["algo"] = fed["algo"].get<std::string>(); this->_algo = args["algo"];}
        // if (fed.find("valid") != fed.end()) { args["valid"] = fed["valid"].get<std::string>(); this->_valid = args["valid"];}
        // if (fed.find("dataset") != fed.end()) { args["dataset"] = fed["dataset"].get<std::string>(); this->_dataset = args["dataset"];}
        // if (fed.find("partition") != fed.end()) { args["partition"] = fed["partition"].get<std::string>(); this->_partition = args["partition"];}
        // if (fed.find("accuracy") != fed.end()) { this->_targetAccuracy = fed["accuracy"].get<double>(); }
        // if (fed.find("stop") != fed.end()) { this->_stop = fed["stop"].get<std::string>(); }
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

    const FedRole role() const { return _role; }

    const std::string gui() const { return _gui; }

    FedNode(std::string nodeUrl, std::string path) {
        PCREATE;
        parseTopology(nodeUrl, path);
    }

    virtual ~FedNode() { PDESTROY; }

    void update() {
        this->_updated = std::time(NULL);
    }

    void consistencyCheck() {

    }

    void parseTopology(std::string nodeUrl, std::string path) {
        // picojson::value data;
        // if (Jericho::FileSystem::readJson(data, path.c_str()) < 0) {
        //     BRED("Failed to read federator path: %s!\n", path.c_str());
        //     exit(1);
        // }
        // BMAG("Fed Node migrating: %s...\n", path.c_str());
        // BMAG("Local url: %s\n", nodeUrl.c_str());

        // std::unordered_map<std::string, std::string> args;

        // picojson::object o = data.get<picojson::object>();
        // if (o.find("nodes") == o.end()) { BRED("Edges not found in %s\n", path.c_str()); exit(1); }
        // picojson::object nodes = o["nodes"].get<picojson::object>();
        // for (picojson::object::const_iterator it = nodes.begin(); it != nodes.end(); it++) {
        //     picojson::object info = it->second.get<picojson::object>();
            
        //     std::string url = "undefined";
        //     std::string role = "undefined";
        //     std::string train;
        //     std::string partition;
        //     std::string dataset;

        //     picojson::object* fed = nullptr;
        //     if (info.find("url") != info.end()) {
        //         url = info["url"].get<std::string>();
        //     }
        //     if (nodeUrl == url) { 
        //         this->id = it->first;
        //         BGRE(" MATCH FOUND \n");
        //         if (info.find("role") != info.end()) {
        //             role = info["role"].get<std::string>();
        //             if (role == "agg") {
        //                 this->_role = FED_ROLE_AGGREGATOR;
        //             } else if (role == "cli") {
        //                 this->_role = FED_ROLE_CLIENT;
        //             } else if (role == "main") {
        //                 this->_role = FED_ROLE_CENTRAL;
        //             }
        //         }
        //         if (info.find("options") != info.end()) {
        //             picojson::object opts = info["options"].get<picojson::object>();

        //             if (opts.find("federation") != opts.end()) {
        //                 std::string fedScript = opts["federation"].get<std::string>();
        //                 picojson::value fed;
        //                 if (Jericho::FileSystem::readJson(fed, fedScript.c_str()) < 0) {
        //                     BRED("Failed to read fed script: %s\n", fedScript.c_str());
        //                     exit(1);
        //                 }
        //                 picojson::object fedO = fed.get<picojson::object>();
        //                 if (parseFederatorLocal(fedO, args) < 0) {
        //                     BRED("Failed to parse federator\n");
        //                     exit(1);
        //                 }
        //             }
        //         }
        //         BYEL("Name: %s, Url: %s\n", it->first.c_str(), url.c_str());
        //         if (o.find("edges") == o.end()) { BRED("Edges not found in %s\n", path.c_str()); }
        //         picojson::object edges = o["edges"].get<picojson::object>();
        //         if (edges[this->id].is<picojson::array>()) {
        //             picojson::array arr = edges[this->id].get<picojson::array>();
        //             for (int i = 0; i < arr.size(); i++) {
        //                 std::string s = nodes[arr[i].get<std::string>()].get<picojson::object>()["url"].get<std::string>();
        //                 _fedNeighbors.push_back(s);
        //             }
        //         } else {
        //             BRED("EDGES IS NOT A JSON ARRAY\n");
        //         }
        //     }
        // }
        // BMAG("Fed Node done migrating.\n");
        // std::vector<std::string> toks = prizm::tokenize(nodeUrl, ':');
        // _host = toks[0];
        // _port = toks[1];

    }

    std::string stateStr() { return serializeStatus(); }

    // const bool quorumMet() const { return _quorum <= _clients.size(); }

    // const std::string model() const { return _model; }

    // void model(std::string model) { _model = model; }

    // const std::vector<std::string> clientUrls() const { return _clients; }

    // std::string dispatch(std::string laten) {
        // pthread_mutex_lock(&ws_mutex);
        // if (_pending.size() == 0) {
        //     for (auto n : _fedNeighbors) {
        //         registerCli(n);
        //     }
        // }

        // std::string url = "undefined";
        // int min = 999;
        // for (auto n : _pending) {
        //     if (n.second < min) {
        //         min = n.second;
        //         url = n.first;
        //     }
        // }
        // if (min != 999) {
        //     BGRE("Dispatching to url: %s\n", url.c_str());
        //     int ct = _pending[url];
        //     ct += 1;
        //     _pending[url] = ct;
        //     pthread_mutex_unlock(&ws_mutex);
        // return url;
        // } else {
        //     BRED("FedNode Dispatch: Did not find a node to dispatch!!!\n");
        //     pthread_mutex_unlock(&ws_mutex);
            // return "127.0.0.1:8081";
        // }
    // }

    // void registerCli(std::string url) {
        // if (prizm::contains_key(_pending, url)) {
        //     int ct = _pending[url];
        //     ct += 1;
        //     _pending[url] = ct;
        // } else {
        //     _pending[url] = 1;
        // }
    // }

    void reset() { 
        // _quorum = 1;
        // lr = 0.001;
        // numClients = 0;
        // _rounds = 3;
        // _round = 0;
        _epochs = 5;
        // targetAccuracy = 95.0;
        _accuracy = 0.0;
        _accumTrain = 0.0;
        _accumLatency = 0.0;
        // _accumRounds = 0;
        _needsUpdate = false;
        // _resource = 0.0;
        _stop = "undefined";
        // optimizer = "adam";
        // algo = "fed-avg";
        // fusion = "fusion.py";
        _queue = {};
        _status = FL2_DORMANT;
    }

    // void queueUpdate(picojson::object o) {
    //     if (_status == FL2_TRAINING) {
    //         _needsUpdate = true;
    //         _updateParams = o;
    //     } else {
    //         this->update(o);
    //     }
    // }

    // void update(picojson::object o) {
    //     if (parseFederatorLocal(o, args) < 0) {
    //         BRED("Failed to update federator\n");
    //     }
    // }

    // void accumTrain(double trainTime) {
    //     _accumTrain += trainTime;
    // }

    // void accumLatency(double latencyTime) {
    //     _accumLatency += latencyTime;
    // }

    // const double finalTrain() const { return _accumTrain; }
    // const double finalRounds() const { return _accumRounds; }
    // const double finalLatency() const { return _accumLatency; }

    // const std::vector<std::string> clients() const { 
        // std::vector<std::string> portsOnly; // for experimental setting, need to change for production
        // for (auto c : _clients) {
        //     std::string::size_type p;
        //     if (p = c.find(':') != std::string::npos) {
        //         std::vector<std::string> toks = prizm::tokenize(c, ':');
        //         portsOnly.push_back(toks[1]);
        //     } else {
        //         portsOnly.push_back(c);
        //     }
        // }
    //     return portsOnly; 
    // }

    // void state(FLStatusType2 type) {
    //     _status = type;
    // }

    // void queue(std::vector<std::string> args) {
    //     _queue.push_back(args);
    // }

    // const int rounds() const { return _rounds; }

    // const int round() const { return _round; }

    // void round(int round) { _round = round; }

    // void rounds(int rounds) { _rounds = rounds; }

    // void accuracy(double acc) { _accuracy = acc; }

    // const std::string stopCond() const { return _stop; } 

    void halt() { _status = FL2_HALTED; }

    void stop() { _status = FL2_STOPPED; }

    void training() { _status = FL2_TRAINING; }

    void train() { _status = FL2_TRAINING; }

    void trained() { _status = FL2_TRAINED; }

    void joined() { _status = FL2_JOINED; }

    // const int quorum() { return _quorum; }

    // const double accuracy() const { return _accuracy; }

    // void finishRound() { _round++; _accumRounds++; }

    // void addNeighbor(std::string hostname) {
        // if (!contains(_fedNeighbors, hostname)) {
        //     _fedNeighbors.push_back(hostname);
        // }
    // }

    // const bool finished() const { return _round >= _rounds; }

    // void addClient(std::string hostname) {
        // if (!contains(_clients, hostname) && !contains(_fedNeighbors, hostname)) {
        //     _clients.push_back(hostname);
        // }
    // }

    // std::vector<std::string> neighbors() {
    //     return _fedNeighbors;
    // }

    // std::vector<std::string> outs() {
    //     return _fedNeighbors;
    // }

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

    virtual void dump() {
        // BCYA("id: %s\n", id.c_str());
        // BCYA("role: %s\n", _parseRole(_role).c_str());
        // if (_role == FED_ROLE_AGGREGATOR) {
            // BCYA("script: %s\n", fusion.c_str());
        // } else if (_role == FED_ROLE_CLIENT) {
        //     BCYA("script: %s\n", _train.c_str());
        // } else if (_role == FED_ROLE_CENTRAL) {
        //     BCYA("script: %s\n", _test.c_str());
        // }
        // BCYA("host: %s\n", _host.c_str());
        // BCYA("port: %s\n", _port.c_str());
        // BCYA("quorum: %i\n", _quorum);
        // BCYA("rounds: %i\n", _rounds);
        // BCYA("lr: %f\n", lr);
        // BCYA("optimizer: %s\n", optimizer.c_str());
        // BCYA("algo: %s\n", algo.c_str());
        // BCYA("fusion: %s\n", fusion.c_str());
        // BCYA("round: %i\n", _round);
        // BCYA("rounds: %i\n", _rounds);
        // BCYA("target-accuracy: %f\n", targetAccuracy);

        // std::string neighborStr = "[";
        // for (auto n : _fedNeighbors) {
        //     neighborStr += n + ", ";
        // }
        // if (neighborStr != "[") {
        //     neighborStr.pop_back(); neighborStr.pop_back();
        // }
        // neighborStr += "]";
        // bold(); cyan();
        // std::cout << "Neighbors: " << neighborStr << std::endl;
        // clearcolor();
    }

};

#endif