#ifndef JERICHO_API_SYS_H_
#define JERICHO_API_SYS_H_

#include "api/api_helper.h"
#include "util/file_system.hpp"

API(Setup, {})
    REQUEST_INFO
    std::string script = router->federator()->script();
    picojson::value data;
    std::string result;
    if (JFS::readJson(data, script.c_str()) < 0) {
        result = "failed to parse script";
    } else {
        result = data.serialize();
    }
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

#endif