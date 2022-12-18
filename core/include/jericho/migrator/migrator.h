#ifndef JERICHO_MIRGRATOR_MIGRATOR_H_
#define JERICHO_MIRGRATOR_MIGRATOR_H_

#include <vector>

#include "server/fetch.h"
#include "server/defs.h"
#include "message/message_buffer.h"
#include "util/file_system.hpp"
#include "prizm/prizm.h"
#include "federate/federator.h"

typedef struct Orchestrator {
    std::string version;
    std::string dbPath;
    std::string fedPath;
    std::string apiPath;
    std::string topPath;
    std::string routerPath;
    std::string environment;
    Federator* federator = nullptr;
    std::vector<std::pair<std::string, std::string>> dbs;
    std::string dir;
    std::vector<std::tuple<std::string, Orchestrator*, std::string, int (*)(std::string, Orchestrator*, std::string)>> migrators;
} Orchestator;

int migrateRouter(std::string nodeUrl, Orchestrator* orch, std::string path) {
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

int migrateFederator(std::string nodeUrl, Orchestrator* orch, std::string path) {
    picojson::value data;
    
    FedNode* fedNode = new FedNode(nodeUrl, path);
    if (orch->federator == nullptr) {
        Federator* fed = new Federator;
        orch->federator = fed;
        orch->federator->addLocal(fedNode);
    } else {
        orch->federator->addLocal(fedNode);
    }

    BYEL("YOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
    fedNode->dump();

    if (fedNode->role() == FED_ROLE_CENTRAL) {
        orch->federator->topology(path.c_str());
    }

    BGRE("migrateFederator: Finished\n");
    return 0;
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

int migrate(std::string nodeUrl, Orchestrator* orch, std::string path) {
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
            orch->migrators.push_back({nodeUrl, orch, it->second.get<std::string>(), &migrateRouter});
        } else if (it->first == "database") {
            char* p = strstr(str, ".json");
            if (p == NULL) { BRED("migrate: Not a json file: %s\n", s.c_str()); return -1; }
            orch->migrators.push_back({nodeUrl, orch, it->second.get<std::string>(), &migrateDbCluster});
        } else if (it->first == "federator") {
            char* p = strstr(str, ".json");
            if (p == NULL) { BRED("migrate: Not a json file: %s\n", s.c_str()); return -1; }
            orch->migrators.push_back({nodeUrl, orch, it->second.get<std::string>(), &migrateFederator});
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