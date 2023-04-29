#ifndef JERICHO_MODULE_H_
#define JERICHO_MODULE_H_

#include "celerity/celerity.h"
#include "util/file_system.hpp"
#include "picojson/picojson.h"
#include "module/migrator.h"
#include "module/dns.h"

class Module {
  public:
    static Celerity* celerity() {
        // celerity = new Celerity(dbname);

        // std::string schema = celerity->serialize();
        // BMAG("SCHEMA: %s\n", schema.c_str());

        // User user;
        // Users users = user.find_by("username", "tankinfranklin").commit(celerity->primary()).as<Users>(); 
        // BBLU("Users size: %i\n", (int)users.size());

        // Artist artist(name_("Jerry Jones"));
        // artist->persist(celerity->primary());
        return nullptr;
    }

    static FedNode* federator(const std::string& script, const std::string& url) {
        picojson::value v;
        if (JFS::readJson(v, script.c_str()) < 0) {
            BRED("Module::federator: Failed to read script '%s'\n", script.c_str());
            return nullptr;
        }
        if (!v.is<picojson::object>()) {
            BRED("Module::federator: Script '%s' should be a JSON object\n", script.c_str());
            return nullptr;
        }
        picojson::object o = v.get<picojson::object>();
        if (o.find("federator") == o.end()) {
            BRED("Module::federator: No federator key detected in JSON object %s\n", v.serialize().c_str());
            return nullptr;
        }
        if (!o["federator"].is<std::string>()) {
            BRED("Module::federator: key 'federator' is not of type string\n");
            return nullptr;
        }    
        std::string fedScript = o["federator"].get<std::string>();
        BMAG("Federator script\n");
        printf("\t%s\n", fedScript.c_str());
        FedNode* node = migrate::federator(fedScript, url); 
        return node;
    }

    static DNS* dns(const std::string& script) {
        picojson::value v;
        if (JFS::readJson(v, script.c_str()) < 0) {
            BRED("Module::DNS: Failed to read script '%s'\n", script.c_str());
            return nullptr;
        }
        if (!v.is<picojson::object>()) {
            BRED("Module::DNS: Script '%s' should be a JSON object\n", script.c_str());
            return nullptr;
        }

        picojson::object o = v.get<picojson::object>();
        return new DNS(o);
    }

};

#endif