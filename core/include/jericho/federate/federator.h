#ifndef FEDERATE_FEDERATOR_H_ 
#define FEDERATE_FEDERATOR_H_   
    
#include <ctime>

#include "prizm/prizm.h"
#include "federate/fed_node.h"
#include "federate/fed_client.h"

class Federator {
  private:
    bool _needsAgg = false;
    int _numClients = 0;
    long long _aggStart = 0;
    long long _aggWait = 10 * 1000;
    int _numRounds = 0;
    int _roundCt = 0;
    int _clientCt = 0;
    bool _active = false;
    FedRole _fedRole = FED_ROLE_NULL;
    std::string _bytes = "undefined";
    int _id;
    FedNode* _local = nullptr;
    std::vector<FedNode*> _fedNodes = {};
    std::unordered_map<std::string, FedNode*> _neighbors = {};
    std::vector<std::string> _connections = {};
    std::string nodeUrl;
    std::string _topology;
    std::unordered_map<std::string, FedNode*> _assignments = {};
    std::vector<std::string> _clients = {};
    std::string _script = "undefined";

  public:

    Federator(int numClients, int numRounds, int secWait, int id) {
        _numClients = numClients;
        _numRounds = numRounds;
        _aggWait = secWait;
        _id = id;
    }

    Federator() {
        BYEL("DEFAULT CONSTRUCTOR FOR FEDERATOR\n");
        write_file("log/debug.log", "constructing federator");
    }

    // ~Federator() {
    //     write_file("log/debug.log", "FUCK THIS SHIT :: Federator");
    // }

    const long long startTime() const { return _aggStart; }

    const long long waitTime() const { return _aggWait; }

    void connect(std::string url) { _connections.push_back(url); }

    std::vector<std::string> connections() const { return _connections; }

    FedNode* local() { return _local; }

    std::string bytes() const { return _bytes; }

    void bytes(std::string bytes) { _bytes = bytes; } 

    const int rounds() const { return _numRounds; }

    void start() { _active = true; _aggStart = std::time(NULL); }

    void stop() { _active = false; }

    void addNode(FedNode* node) { _fedNodes.push_back(node); }

    const int id() const { return _id; }

    const int round() const { return _roundCt; }

    const int clientCt() const { return _clientCt; }

    void incClientCt() { _clientCt++; }

    void clientCt(int ct) { _clientCt = ct; } 

    const bool active() const { return _active; }

    const std::string script() const { return _script; }

    void finishRound() {
        _roundCt++;
        _aggStart = std::time(NULL);
        _clientCt = 0;
        if (_roundCt == _numRounds) {
            BGRE("FEDERATION COMPLETED\n");
            _active = false;
        }
    }


    void dump() {
        BCYA("===========================\n");
        BCYA("LOCAL:\n");
        _local->dump();
        BCYA("NEIGHBORS:\n");
        for (auto n : _neighbors) {
            BCYA("%s ", n.first.c_str());
        }
        BCYA("\n");
        BCYA("TOPOLOGY:\n");
        for (auto fed : _fedNodes) {
            fed->dump();
        }
        BCYA("---------------------------\n");
        for (auto asg : _assignments) {
            BYEL("%s\n", asg.first.c_str());
            asg.second->dump();
        }
        BCYA("===========================\n");
    }

    void addLocal(FedNode* node) {
        _local = node;
    }

    std::string dispatch(std::string hostname, double time_elapsed) {
        BYEL("DO NOTHING\n");
        std::string url = "undefined";
        for (auto asg : _assignments) {
            if (asg.second->clients().size() == 0) {
                url = asg.first;
            }
        }
        return url;
    }

    void poll(std::string hostname, System* router, std::string url, MessageCallback callback, Benchmark* bm);

    void train(std::string hostname, System* router, std::string url, MessageCallback callback, Benchmark* bm);

    int parseFederator(FedNode* fedNode, picojson::object fed, std::unordered_map<std::string, std::string>& args) {
        BYEL("Parsing federator...\n");
        if (fed.find("lr") != fed.end()) { args["lr"] = std::to_string(fed["lr"].get<double>()); fedNode->lr = std::stod(args["lr"]); }
        if (fed.find("optimizer") != fed.end()) { args["optimizer"] = fed["optimizer"].get<std::string>(); fedNode->optimizer = args["optimizer"]; }
        if (fed.find("fusion") != fed.end()) { args["fusion"] = fed["fusion"].get<std::string>(); fedNode->fusion = args["fusion"];}
        if (fed.find("train") != fed.end()) { args["train"] = fed["train"].get<std::string>(); fedNode->_train = args["train"];}
        if (fed.find("test") != fed.end()) { args["test"] = fed["test"].get<std::string>(); fedNode->_test = args["test"];}
        if (fed.find("quorum") != fed.end()) { args["quorum"] = std::to_string((int)fed["quorum"].get<double>()); fedNode->_quorum = std::stoi(args["quorum"]);}
        if (fed.find("algo") != fed.end()) { args["algo"] = fed["algo"].get<std::string>(); fedNode->algo = args["algo"];}
        if (fed.find("valid") != fed.end()) { args["valid"] = fed["valid"].get<std::string>(); fedNode->_valid = args["valid"];}
        if (fed.find("dataset") != fed.end()) { args["dataset"] = fed["dataset"].get<std::string>(); fedNode->dataset = args["dataset"];}
        if (fed.find("resource") != fed.end()) { fedNode->_resource = fed["resource"].get<double>(); }
        if (fed.find("rounds") != fed.end()) { fedNode->_rounds = (int)fed["rounds"].get<double>(); }
        if (fed.find("accuracy") != fed.end()) { fedNode->targetAccuracy = fed["accuracy"].get<double>(); }
        if (fed.find("stop") != fed.end()) { fedNode->_stop = fed["stop"].get<std::string>(); }
        BYEL("DONE FEDERATING\n");
        return 0;
    }

    int topology(const char* path) {
        _script = std::string(path);
        picojson::value data;
        if (Jericho::FileSystem::readJson(data, path) < 0) {
            BRED("Failed to parse Json file: %s\n", path);
            return -1;
        }
        BYEL("Federator class parsing topology information...\n");
        picojson::object o = data.get<picojson::object>();
        if (o.find("nodes") == o.end()) { BRED("Topology file does not have nodes attribute!\n"); return -1; }
        if (o.find("edges") == o.end()) { BRED("Topology file does not have edges attribute!\n"); return -1; }
        if (o.find("type") != o.end()) {
            _topology = o["type"].get<std::string>();
            //  BRED("Topology file does not have type attribute!\n"); return -1;
        }
        picojson::object edges = o["edges"].get<picojson::object>();
        picojson::object nodes = o["nodes"].get<picojson::object>();
        for (picojson::object::const_iterator it = nodes.begin(); it != nodes.end(); it++) {
            picojson::object node = it->second.get<picojson::object>();
            std::string id = it->first;
            std::string nodeUrl = node["url"].get<std::string>();
            std::string role = node["role"].get<std::string>();
            picojson::object opts;
            FedNode* fedNode = new FedNode(id, nodeUrl, role, {});
            if (fedNode == nullptr || fedNode == NULL) {
                BRED("FED NODE IS NULL\n");
                exit(1);
            }
            std::unordered_map<std::string, std::string> args;
            if (node.find("options") != node.end()) {
                opts = node["options"].get<picojson::object>();
                if (opts["federation"].is<picojson::object>()) {
                    picojson::object fed = opts["federation"].get<picojson::object>();
                    if (parseFederator(fedNode, fed, args) < 0) {
                        BRED("Failed to parse federator\n");
                        return -1;
                    }
                } else {
                    std::string fedFile = opts["federation"].get<std::string>();
                    picojson::value data2;
                    if (Jericho::FileSystem::readJson(data2, fedFile.c_str()) < 0) { return -1; }
                    picojson::object fed = data2.get<picojson::object>();
                    if (parseFederator(fedNode, fed, args) < 0) {
                        BRED("Failed to parse federator\n");
                        return -1;
                    }
                }
            }
            if (role != "cli") {
                for (picojson::object::const_iterator edge = edges.begin(); edge != edges.end(); edge++) {
                    if (edge->first == id && edge->second.is<picojson::array>()) {
                        picojson::array arr = edge->second.get<picojson::array>();
                        for (int i = 0; i < arr.size(); i++) {
                            std::string s = arr[i].get<std::string>();
                            if (fedNode == nullptr || fedNode == NULL) {
                                BRED("NULL NODE\n");
                                exit(1);
                            }
                            fedNode->dump();
                            fedNode->addNeighbor(nodeUrl);
                            // if (_neighbors.find(s) != _neighbors.end()) {
                            //     BYEL("THIS IS WHY, NEED A MULTI-MAP\n");
                            // }
                            BYEL("ERRR...\n");
                            _neighbors[nodeUrl] = fedNode; // this seg faults and its fucking dumb why
                            BYEL("Finished migrating topology\n");
                        }
                    }
                }
                _fedNodes.push_back(fedNode);
                if (role != "main") {
                    _assignments[nodeUrl] = fedNode;
                }
                BRED("Finished migrating topology\n");
            }
        }
        BBLU("Finished migrating topology\n");
        return 0;
    }
};

#endif