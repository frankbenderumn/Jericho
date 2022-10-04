#include "server/iris.h"

#include <unordered_map>

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

void replace(std::string& str, std::string sub, std::string rep) {
    std::string::size_type p = str.find(sub);
    if (p != std::string::npos) {
        str.replace(p, sub.size(), rep);
    }
}

int loc(std::string& str, std::string sub) {
    std::string::size_type p = str.find(sub);
    if (p != std::string::npos) {
        return (int)p;
    }
    return -1;
}

static std::unordered_map<std::string, std::string> symbol_table;

namespace iris {
    void inject(std::string symbols, std::string& file) {
        // std::stringstream ss(symbols);
        // std::string word;
        // std::vector<std::string> words;
        // while(std::getline(ss, word, ';')) {
        //     words.push_back(word);
        // }
        // for (auto w : word) {
        //     std::string symbol;
        //     std::string value;
        //     std::string::size_type p = w.find("=");
        //     if (p != std::string::npos) {
        //         symbol = w.substr(0, p);
        //         value = w.substr(p+1, w.size());
        //         printf("Symbol is: %s\n", symbol.c_str());
        //         printf("Value is: %s\n", value.c_str());
        //         symbol_table[symbol] = value;
        //         // add trim function?
        //     }
        // }
    }

    void interpret(std::string& file) {
        // BGRE("INTERPRETTING...\n");
        LEXES ls = scan(file, std::regex("\\{\\{.+\\}\\}+"));
        if (ls.size() > 0) {
            for (auto lex : ls) {
                // printf("Lex found: %s - %i\n", lex.first.c_str(), lex.second);
                replace(file, lex.first, std::string("replacement"));
            }
        } else {
            printf("No lexes found\n");
        }
        // printf("NEW FILE IS: %s\n", file.c_str());
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
