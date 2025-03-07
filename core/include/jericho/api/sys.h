#ifndef JERICHO_API_SYS_H_
#define JERICHO_API_SYS_H_

#include "api/api_helper.h"
#include "util/file_system.hpp"

API(SSetup, {})
    REQUEST_INFO
    // std::string script = router->federator()->script();
    picojson::value data;
    std::string result;
    // if (JFS::readJson(data, script.c_str()) < 0) {
    //     result = "failed to parse script";
    // } else {
    //     result = data.serialize();
    // }
    return result;
}

API(SaveScript, {})
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    BMAG("apiSaveScript\n");
    picojson::value data;
    std::string name = "demo-gui";
    std::string cerr = picojson::parse(data, content);
    if (!cerr.empty()) {
        BRED("SaveScript: Failed to parse Json!\n");
    } else {
        picojson::object o = data.get<picojson::object>();
        if (o.find("name") != o.end()) {
            name = o["name"].get<std::string>();
        }
    }
    std::string path = "./config/" + name + ".json";
    BMAG("%s\n", content.c_str());
    JFS::write(path.c_str(), data.serialize(true), true);
    return "Script saved to backend";
}

API(ListScripts, {})
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    BMAG("apiLoadScript\n");
    std::vector<std::string> files = JFS::getDir("./config");
    picojson::array o;
    for (auto f : files) {
        BMAG("File: %s\n", f.c_str());
        if (f.find("-gui.json") != std::string::npos || f == "demo.json") {
            o.push_back(picojson::value(f));
        }
    }
    picojson::value v(o);
    std::string result = v.serialize();
    return result;
}

API(LoadScript, {})
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    BMAG("apiLoadScript\n");
    BMAG("%s\n", content.c_str());
    picojson::value data;
    std::string result;
    std::string path = "./config/" + content;
    if (JFS::readJson(data, path.c_str()) < 0) {
        result = "failed to parse script";
    } else {
        result = data.serialize();
    }
    return result;
}

API(LaunchCluster, {})
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    BBLU("Launching script! Default is orchestrator.json\n");

    // Orchestrator* orch = new Orchestrator;
    // if (migrate(host+":"+portStr, orch, script) < 0) {
    //     BRED("Main.cc: Failed to migrate!: %s\n", script.c_str());
    //     return 1;
    // }
    // if (orch->federator == nullptr) {
    //     BMAG("Orch federator is null somehow\n");
    //     exit(1);
    // }
    // orch->federator->dump();
    // FedRole role = orch->federator->local()->role();
    // if (role == FED_ROLE_CENTRAL || role == FED_ROLE_AGGREGATOR) {
    //     sys->cluster()->boss()->configDir("agg");
    // }
    // sys->federator(orch->federator);
    // write_file("./log/debug.log", "Hello friend");

    return JsonResponse::success(200, "Launching cluster");
}

#endif