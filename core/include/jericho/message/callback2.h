#ifndef MESSAGE_CALLBACK2_H_
#define MESSAGE_CALLBACK2_H_

#include <string>

#include "util/trace.h"

struct Callback {
    std::string url;
    std::string endpoint;
    std::string type;
    bool satellite;
    std::string fn;
    int count;
    int limit;
    bool resolved = 0;
    Callback() { PCREATE; }
    
    Callback(std::string url, std::string endpoint, std::string type, bool satellite, std::string fn, int count, int limit) {
        PCREATE;
        this->url = url;
        this->endpoint = endpoint;
        this->type = type;
        this->satellite = satellite;
        this->fn = fn;
        this->count = count;
        this->limit = limit;
    }

    Callback(std::unordered_map<std::string, std::string> map) {
        PCREATE;
        this->url = map["Callback-Url"];
        this->endpoint = map["Callback-Endpoint"]; 
        this->type = map["Callback-Type"];
        this->satellite = std::stoi(map["Callback-Satellite"]);
        this->fn = map["Callback-Fn"];        
        this->count = std::stoi(map["Callback-Count"]);
        this->limit = std::stoi(map["Callback-Limit"]);
        this->resolved = (map["Callback-Resolved"] == "1");
    }

    void serialize(std::unordered_map<std::string, std::string>& map) {
        map["Callback-Url"] = this->url;
        map["Callback-Endpoint"] = this->endpoint; 
        map["Callback-Type"] = this->type;
        map["Callback-Satellite"] = std::to_string(this->satellite);
        map["Callback-Fn"] = this->fn;
        map["Callback-Count"] = std::to_string(this->count);
        map["Callback-Limit"] = std::to_string(this->limit);
        map["Callback-Resolved"] = std::to_string(this->resolved);  
    }

    ~Callback() {
        PDESTROY;
    }
};

#endif