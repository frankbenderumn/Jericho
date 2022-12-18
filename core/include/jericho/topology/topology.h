#ifndef TOPOLOGY_TOPOLOGY_H_
#define TOPOLOGY_TOPOLOGY_H_

#include <string>
#include <vector>

#include "cluster/cluster_node.h"
#include "picojson.h"

enum TopologyType {
    TOPO_NULL,
    TOPO_RING,
    TOPO_STAR,
    TOPO_TREE,
    TOPO_MESH,
    TOPO_BUS,
    TOPO_GRAPH,
    TOPO_HYPERGRAPH,
    TOPO_DAG,
    TOPO_DCG,
    TOPO_CUSTOM,
    TOPO_AUTO,
    TOPO_DYNAMIC
};

#ifndef TOPOLOGY_EX
#define TOPOLOGY_EX
// may need to change flags to array of objects rather than strings
    std::string TOPO = R"({
    "topology": "graph",
    "flags": ["same-host"],
    "degrees": 1,
    "nodes": [
        {"host": "8080", "role": "agg"},
        {"host": "8081"},
        {"host": "8082"},
        {"host": "8083"},
        {"host": "8084"},
        {"host": "8086"},
        {"host": "8087"},
        {"host": "8088"},
        {"host": "8089"},
        {"host": "8090"},
        {"host": "8091"},
        {"host": "8092"},
        {"host": "8093"},
        {"host": "8094"},
        {"host": "8095"},
        {"host": "8096"},
        {"host": "8097"},
        {"host": "8098"},
        {"host": "8099"},
        {"host": "8100"}
    ],
    "edges": [
        {"host": "8080", "edge": []},
        {"host": "8081", "edge": []},
        {"host": "8082", "edge": []},
        {"host": "8083", "edge": []},
        {"host": "8084", "edge": []},
        {"host": "8086", "edge": []},
        {"host": "8087", "edge": []},
        {"host": "8088", "edge": []},
        {"host": "8089", "edge": []},
        {"host": "8090", "edge": []},
        {"host": "8091", "edge": []},
        {"host": "8092", "edge": []},
        {"host": "8093", "edge": []},
        {"host": "8094", "edge": []},
        {"host": "8095", "edge": []},
        {"host": "8096", "edge": []},
        {"host": "8097", "edge": []},
        {"host": "8098", "edge": []},
        {"host": "8099", "edge": []},
        {"host": "8100", "edge": []}
    ]
})";
#endif

class Topology {
    ClusterNode* _root;
    TopologyType _type;
    std::string _host;
    std::string _port;

  public:
    Topology(ClusterNode* root) {
        _root = root;
    }

    TopologyType parseType(std::string s) {
        if (s == "ring") {
            return TOPO_RING;
        } else if (s == "mesh") {
            return TOPO_MESH;
        } else if (s == "tree") {
            return TOPO_TREE;
        } else if (s == "star") {
            return TOPO_STAR;
        } else if (s == "graph") {
            return TOPO_GRAPH;
        } else if (s == "hypergraph") {
            return TOPO_HYPERGRAPH;
        } else if (s == "dcg") {
            return TOPO_DCG;
        } else if (s == "dag") {
            return TOPO_DAG;
        } else if (s == "bus" ) {
            return TOPO_BUS;
        } else if (s == "auto") {
            return TOPO_AUTO;
        } else if (s == "custom") {
            return TOPO_CUSTOM;
        } else if (s == "dynamic") {
            return TOPO_DYNAMIC;
        }
        return TOPO_NULL;
    }

    int migrate(std::string script, std::string& err) {
        std::ifstream nodes(script.c_str());
        std::stringstream buf;
        buf << nodes.rdbuf();
        std::string json = buf.str();
        BCYA("Json is: %s\n", json.c_str());
        picojson::value data;
        std::string err = picojson::parse(data, json);
        if (!err.empty()) {
            std::cerr << err << std::endl;
            BRED("Failed to parse json file: %s\n", path.c_str());
            return -1;
        } else {
            BGRE("Json data parsed for: %s\n", path.c_str());
        }

        if (!data.is<picojson::object>()) {
            BRED("Failed to migrate topology. Not a JSON object!\n");
            return -1;
        }

        picojson::object o = data.get<picojson::object>();

        if (o.find("nodes") == o.end()) {
            BRED("Failed to migrate topology. Nodes not specified!\n");
            return -1;
        }

        if (o.find("edges") == o.end()) {
            BRED("Failed to migrate topology. Nodes not specified!\n");
            return -1;
        }

        if (o.find("topology") != o.end()) {
            picojson::value v = o["topology"];
            _type = parseType(v.get<std::string>());
        } else {
            _type = TOPO_GRAPH;
        }

        if (o.find("flags") != o.end()) {
            picojson::value val = o["flags"];
            if (val.is<picojson::array>()) {
                picojson::array arr = val.get<picojson::array>();
                for (int i = 0; i < arr.size(); i++) {
                    if (!arr[i].is<std::string>()) {
                        BRED("Failed to migrate topology. Flag array value not a string!\n")
                    }

                    std::string s = arr[i].get<std::string>();

                    if (s == "same-host") {
                        _host = "127.0.0.1";
                    } else {
                        BYEL("Migration error: Unknown celerity attribute encountered\n");
                    }
                }
            } else {
                BRED("Failed to migrate topology. Flag value not an array!\n");
            }
        }

        for (picojson::object::const_iterator it = o.begin(); it != o.end(); it++) {

        }
    }

};

#endif