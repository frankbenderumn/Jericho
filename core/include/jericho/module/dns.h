#ifndef JERICHO_MODULE_DNS_H_
#define JERICHO_MODULE_DNS_H_

#include "util/trace.h"
#include "prizm2/prizm.h"
#include "picojson/picojson.h"

class DNS {
    std::unordered_map<std::string, std::string> _lookup;
  public:
    DNS(const picojson::object& obj) {
        PCREATE;
        for (const auto& el : obj) {
            _lookup[el.first] = el.second.get<std::string>();
        }
    }

    ~DNS() { PDESTROY; }

    bool lookup(std::string& val, const std::string& domain) {
        if (!prizm::contains_key(_lookup, domain)) {
            BRED("DNS::lookup: Domain '%s' is not in lookup table!\n", domain.c_str());
            return false;
        }
        val = _lookup[domain];
        return true;
    }
};

#endif