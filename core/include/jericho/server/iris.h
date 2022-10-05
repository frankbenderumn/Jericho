#ifndef JERICHO_SERVER_IRIS_H_
#define JERICHO_SERVER_IRIS_H_

#include <string>
#include <regex>

#include <deque>
#include <regex>

#include "prizm/prizm.h"

#ifndef LEX
#define LEX std::pair<std::string, int>
#endif

#ifndef LEXES
#define LEXES std::deque<std::pair<std::string, int>>
#endif

#ifndef TOKS
#define TOKS std::deque<std::string>
#endif

LEXES scan(std::string substr, std::regex rgx);

bool validate(std::string substr, std::regex rgx);

void replace(std::string& str, std::string sub, std::string rep);

int loc(std::string& str, std::string sub);


namespace iris {
    void interpret(std::string& file);
    void inject(std::string symbols, std::string& file);
}

#endif