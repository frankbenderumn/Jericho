#ifndef JERICHO_MIRGRATOR_MIGRATOR_H_
#define JERICHO_MIRGRATOR_MIGRATOR_H_

#include <vector>

#include "server/fetch.h"
#include "server/defs.h"
#include "message/message.h"
#include "util/file_system.hpp"
#include "prizm/prizm.h"
#include "federator/fl.h"

typedef struct Orchestrator {
    std::string version;
    std::string dbPath;
    std::string fedPath;
    std::string apiPath;
    std::string topPath;
    std::string routerPath;
    std::string environment;
    // Federator* federator = nullptr;
    std::vector<std::pair<std::string, std::string>> dbs;
    std::string dir;
    std::vector<std::tuple<std::string, Orchestrator*, std::string, int (*)(std::string, Orchestrator*, std::string)>> migrators;
    Orchestrator() { PCREATE; }
    ~Orchestrator() { PDESTROY; }
} Orchestator;

int migrateSystem(std::string nodeUrl, Orchestrator* orch, std::string path) {
    picojson::value data;
    if (Jericho::FileSystem::readJson(data, path.c_str()) < 0) return -1;
    BMAG("Migrating router %s...\n", path.c_str());
    picojson::object o = data.get<picojson::object>();
    if (o.find("routes") == o.end()) { BRED("routes not found in %s\n", path.c_str()); return -1; }
    picojson::object routes = o["routes"].get<picojson::object>();
    for (picojson::object::const_iterator it = routes.begin(); it != routes.end(); it++) {
        picojson::object info = it->second.get<picojson::object>();
        std::string type = "sys";
        if (info.find("type") != info.end()) {
            type = prizm::lowercase(info["type"].get<std::string>());
        }
        BYEL("Name: %s, Type: %s\n", it->first.c_str(), type.c_str());
    }
    return 0;
}

int migrateDbCluster(std::string nodeUrl, Orchestrator* orch, std::string path) {
    picojson::value data;
    if (Jericho::FileSystem::readJson(data, path.c_str()) < 0) return -1;
    BMAG("Migrating cluster %s...\n", path.c_str());
    picojson::object o = data.get<picojson::object>();
    if (o.find("cluster") != o.end()) {
        picojson::object cluster = o["cluster"].get<picojson::object>();
        for (picojson::object::const_iterator it = cluster.begin(); it != cluster.end(); it++) {
            if (it->first == "local") continue;
            picojson::object k = it->second.get<picojson::object>();
            BYEL("Name: %s, Url: %s\n", it->first.c_str(), k["url"].get<std::string>().c_str());
            orch->dbs.push_back({k["url"].get<std::string>(), it->first});
        }
    } else {
        BRED("Unknown database setup detected: %s\n", path.c_str());
        return -1;
    }
    return 0;
}

namespace migrate {

    // may be a good idea to invent a JSON schema validator (maybe like pydantic)
    FedNode* federator(std::string path, std::string url) {
        FedNodeFactory* factory = new FedNodeFactory;
        picojson::value data;

        if (JFS::readJson(data, path.c_str()) < 0) {
            BRED("migrate::federator: File '%s' does not exist!\n", path.c_str());
            return nullptr;        
        }

        if (!data.is<picojson::object>()) {
            BRED("migrate::federator: File '%s' is not a JSON object!\n", path.c_str());
            return nullptr;
        }

        picojson::object o = data.get<picojson::object>();

        if (o.find("nodes") == o.end()) {
            BRED("migrate::federator: Federator object does not have 'nodes' key!\n");
            return nullptr;
        } else if (!o["nodes"].is<picojson::object>()) {
            BRED("migrate::federator: Federator key 'nodes' is not of type object!\n");
            return nullptr;
        }

        if (o.find("name") == o.end()) {
            BRED("migrate::federator: Federator object does not have 'name' key!\n");
            return nullptr;
        } else if (!o["name"].is<std::string>()) {
            BRED("migrate::federator: Federator key 'name' is not of type string!\n");
            return nullptr;
        }

        if (o.find("edges") == o.end()) {
            BRED("migrate::federator: Federator object does not have 'edges' key!\n");
            return nullptr;
        } else if (!o["edges"].is<picojson::object>()) {
            BRED("migrate::federator: Federator key 'edges' is not of type object!\n");
            return nullptr;
        }

        const picojson::object& nodes = o["nodes"].get<picojson::object>();
        const picojson::object& edges = o["edges"].get<picojson::object>();
        std::string name = o["name"].get<std::string>();

        std::map<std::string, std::string> key_to_url_map;
        std::map<std::string, std::string> key_to_config_map;
        std::map<std::string, std::vector<std::string>> parent_map;
        std::map<std::string, std::vector<std::string>> child_map;
        std::string target_key;
        std::string target_role;

        for (const auto& it : nodes) {
            picojson::object node;
            if (!it.second.is<picojson::object>()) {
                BRED("migrate::federator: Node is not an object!\n");
                return nullptr;
            }
            node = it.second.get<picojson::object>();
            if (node.find("url") == node.end()) {
                BRED("migrate::federator: Federator Node key '%s' not found!\n", "url");
                return nullptr;
            } else if (!node["url"].is<std::string>()) {
                BRED("migrate::federator: Federator Node key '%s' not of type '%s'!\n", "url", "string");
                return nullptr;
            }
            if (node.find("role") == node.end()) {
                BRED("migrate::federator: Federator Node key '%s' not found!\n", "role");
                return nullptr;
            } else if (!node["role"].is<std::string>()) {
                BRED("migrate::federator: Federator Node key '%s' not of type '%s'!\n", "role", "string");
                return nullptr;
            }
            if (node.find("options") == node.end()) {
                BRED("migrate::federator: Federator Node key '%s' not found!\n", "options");
                return nullptr;                
            } else if (!node["options"].is<picojson::object>()) {
                BRED("migrate::federator: Federator Node key '%s' not of type '%s'!\n", "options", "object");
                return nullptr;
            }
            std::string role = node["role"].get<std::string>();
            std::string _url = node["url"].get<std::string>();
            if (_url == url) {
                target_key = it.first;
                target_role = role;
            }
            // no check on keys for options params for now (really need a schema validator). Not the most DRY approach rn
            std::string config = node["options"].get<picojson::object>()["federation"].get<std::string>();
            key_to_url_map[it.first] = _url;
            key_to_config_map[it.first] = config;
        }

        if (target_key.empty()) {
            BRED("URL '%s' not found in federator!\n", url.c_str());
            return nullptr;
        }

        for (auto& edge : edges) {
            if (!edge.second.is<picojson::array>()) {
                BRED("migrate::federator: Federator Edge value not of type '%s'!\n", "array");
                return nullptr;
            }
            picojson::array arr = edge.second.get<picojson::array>();
            std::vector<std::string> edge_urls;
            for (int i = 0; i < arr.size(); i++) {
                edge_urls.push_back(key_to_url_map[arr[i].get<std::string>()]);
                child_map[arr[i].get<std::string>()].push_back(key_to_url_map[edge.first]);
            }
            parent_map[edge.first] = edge_urls;
        }

        FedNode* node = factory->create(key_to_url_map[target_key], target_role, key_to_config_map[target_key], parent_map[target_key], child_map[target_key]);
        delete factory;
        return node;
    }

}

int migrateTopology(std::string nodeUrl, Orchestrator* orch, std::string path) {
    // picojson::value data;
    // if (Jericho::FileSystem::readJson(data, path.c_str()) < 0) return -1;
    // BMAG("Migrating topology %s...\n", path.c_str());
    // BMAG("Local url: %s\n", nodeUrl.c_str());
    // picojson::object o = data.get<picojson::object>();
    // if (o.find("nodes") == o.end()) { BRED("nodes not found in %s\n", path.c_str()); return -1; }
    // picojson::object nodes = o["nodes"].get<picojson::object>();
    // for (picojson::object::const_iterator it = nodes.begin(); it != nodes.end(); it++) {
    //     picojson::object info = it->second.get<picojson::object>();
    //     std::string url = "undefined";
    //     if (info.find("url") != info.end()) {
    //         url = prizm::lowercase(info["url"].get<std::string>());
    //     }
    //     BYEL("Name: %s, Url: %s\n", it->first.c_str(), url.c_str());
    // }
    // if (o.find("edges") == o.end()) { BRED("nodes not found in %s\n", path.c_str()); }
    // picojson::object edges = o["edges"].get<picojson::object>();
    // for (picojson::object::const_iterator it = edges.begin(); it != edges.end(); it++) {
    //     picojson::array edge = it->second.get<picojson::array>();
    //     std::string serialEdge = "[";
    //     for (int i = 0; i < edge.size(); i++) {
    //         serialEdge += edge[i].get<std::string>() + ", ";
    //     }
    //     if (serialEdge != "[") {
    //         serialEdge.pop_back(); serialEdge.pop_back();
    //     }
    //     serialEdge += "]";
    //     BYEL("Name: %s, Edges: %s\n", it->first.c_str(), serialEdge.c_str());
    // }
    return 0;
}

int migration(std::string nodeUrl, Orchestrator* orch, std::string path) {
    std::vector<std::string> arr = prizm::tokenize(path, '/');
    std::string file = arr[arr.size() - 1];
    BYEL("FILE: \n\n\n\n%s\n\n\n\n", file.c_str());
    std::vector<std::string> arr2 = prizm::tokenize(file, '.');
    std::string ext = arr2[arr2.size() - 1];
    if (ext != "json") {
        BRED("Invalid file type provided. Json required... for now.\n");
        return -1;
    }

    picojson::value data;
    if (Jericho::FileSystem::readJson(data, path.c_str()) < 0) {
        BRED("migrate: Failed to read Json file\n");
        return -1;
    }

    picojson::object o = data.get<picojson::object>();
    for (picojson::object::const_iterator it = o.begin(); it != o.end(); it++) {
        char str[it->first.size()];
        std::string s;
        if (it->second.is<std::string>()) {
            s = it->second.get<std::string>();
            BYEL("PATH: %s\n", s.c_str());
            strncpy(str, s.data(), s.size());
        }
        if (it->first == "version") {
            orch->version = o["version"].get<std::string>();
        } else if (it->first == "router") {
            char* p = strstr(str, ".json");
            if (p == NULL) { BRED("migrate: Not a json file: %s\n", s.c_str()); return -1; }
            orch->migrators.push_back({nodeUrl, orch, it->second.get<std::string>(), &migrateSystem});
        } else if (it->first == "database") {
            char* p = strstr(str, ".json");
            if (p == NULL) { BRED("migrate: Not a json file: %s\n", s.c_str()); return -1; }
            orch->migrators.push_back({nodeUrl, orch, it->second.get<std::string>(), &migrateDbCluster});
        // } else if (it->first == "federator") {
        //     char* p = strstr(str, ".json");
        //     if (p == NULL) { BRED("migrate: Not a json file: %s\n", s.c_str()); return -1; }
        //     orch->migrators.push_back({nodeUrl, orch, it->second.get<std::string>(), &migrateFederator});
        } else if (it->first == "topology") {
            char* p = strstr(str, ".json");
            if (p == NULL) { BRED("migrate: Not a json file: %s\n", s.c_str()); return -1; }
            orch->migrators.push_back({nodeUrl, orch, it->second.get<std::string>(), &migrateTopology});
        } else if (it->first == "environment") {
            orch->environment = it->second.get<std::string>();
        } else if (it->first == "directory") {
            orch->dir = it->second.get<std::string>();
        } else if (it->first == "api") {
            orch->apiPath = it->second.get<std::string>();
        } else {
            BRED("migrate: Undefined orchestration attribute\n");
            // return -1;
        }
    }

    std::vector<std::string> files;
    if (orch->apiPath != "")
        files = Jericho::FileSystem::getDir(orch->apiPath);

    for (auto m : orch->migrators) {
        if (std::get<3>(m)(std::get<0>(m), std::get<1>(m), std::get<2>(m)) < 0) {
            BRED("migrate: Failed to migrate: %s\n", std::get<2>(m).c_str());
            return -1;
        }
    }

    BGRE("Finished migration\n");

    BYEL("This is impossible\n");

    return 0;
}

#endif