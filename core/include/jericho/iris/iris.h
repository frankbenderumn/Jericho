#ifndef JERICHO_SERVER_IRIS_H_
#define JERICHO_SERVER_IRIS_H_

#include <string>
#include <regex>

#include <deque>
#include <regex>

#include "prizm/prizm.h"
#include "system/system.h"
#include "iris/defs.h"

LEXES scan(std::string substr, std::regex rgx);

bool validate(std::string substr, std::regex rgx);

void replace(std::string& str, std::string sub, std::string rep);

int loc(std::string& str, std::string sub);

typedef std::vector<std::pair<std::string, std::vector<std::string>>> Methods;

namespace iris {
    // void interpret(std::string& file);
    
    void inject(std::string symbols, std::string& file);

    std::string parseTemp(std::string dir, LEX lex);

    std::string parseForm(LEX lex);

    std::string parseLink(Router* router, LEX lex);

    std::string interpret(Router* router, std::string file);

    // std::string interpret(std::string file);

    void replace(std::string &str, std::string sub, std::string rep);

    static bool IS_KV = false;

    Methods parseMethods(std::string lex);

    void execMethods(std::vector<std::pair<std::string, std::vector<std::string>>> methods, std::string dir, std::string insName, std::string insVal, bool kv = false);

    Deltas ormScript(std::string script);

    bool checkMeta(std::string dir, std::string path);

    void createMeta(std::string dir, std::string path);

    void clearCache(std::string dir);

    void cache(std::string dir, std::string name, std::string contents);

    std::string checkCache(std::string dir, std::string name);

    LEXES scanzz(std::string substr, std::regex rgx);

    // std::vector<std::string> iris_entities;

}

#endif