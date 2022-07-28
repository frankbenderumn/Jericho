#ifndef JERICHO_SERVER_IRIS_H_
#define JERICHO_SERVER_IRIS_H_

#include <string>
#include <regex>

#include <deque>
#include <regex>

#define LEX std::pair<std::string, int>
#define LEXES std::deque<std::pair<std::string, int>>
#define TOKS std::deque<std::string>
#define MONEY_RGX std::regex("^$[0-9]{1,3}(?:,[0-9]{3})*\\.[0-9]{2}$")

LEXES scan(std::string substr, std::regex rgx) {
    LEXES result;
    std::regex_iterator<std::string::iterator> it(substr.begin(), substr.end(), rgx);
    std::regex_iterator<std::string::iterator> end;
    for (; it != end; ++it) {
        LEX l({it->str(), it->position()});
        result.push_back(l);
    }
    return result;
}

bool validate(std::string substr, std::regex rgx) {
    std::regex_iterator<std::string::iterator> it(substr.begin(), substr.end(), rgx);
    std::regex_iterator<std::string::iterator> end;
    if (std::distance(it, end) != 0) {
        return true;
    }
    return false;
}

    std::string::size_type p = orig.find(sub);
    if (p != std::string::npos) {
        orig.replace(p, sub.size(), rep);
    }


namespace iris {
    void interpret(std::string& file) {
    // std::stringstream ss(file);
    // std::string word, push_field("");
    // bool no_quotes = true;
    // while (std::getline(ss, word, ',')) {
    //     if (static_cast<size_t>(std::count(word.begin(), word.end(), '"')) % 2 != 0) {
    //         no_quotes = !no_quotes;
    //     }

    //     push_field += word + (no_quotes ? "" : ",");

    //     if (no_quotes) {
    //         r.push_back(push_field);
    //         push_field.clear();
    //     }
    // }
    // for (auto w : r) {
    //     printf("tok -> %s\n", w.c_str());
    // }
    }
}

#endif