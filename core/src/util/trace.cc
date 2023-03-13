#include "util/trace.h"

namespace prizm {

std::pair<int, int> substrcmp(std::string s1, std::string s2) {
    // WHI("substrcmp: %s, %s\n", s1.c_str(), s2.c_str());
    int result = 0;
    bool subset = false;
    for (int i = 0; i < s1.size(); i++) {
        for (int j = 0; j < s2.size(); j++) {
            char c1 = s1.at(i);
            char c2 = s2.at(j);
            if (c1 == c2) {
                result++;
                subset = true;
                if (i < s1.size() - 1) {
                    i++;
                } else {
                    // BRED("Under substrcmp test: %s, %s\n", s1.c_str(), s2.substr(0, i + 1).c_str());
                    // BRED("Over substrcmp test: %s, %s\n", s1.substr(0, i + 1).c_str(), s2.c_str());

                    if (s2.size() > i + 1 && s2.substr(0, i + 1) == s1) {
                        // BGRE("Under sub detected\n");
                        return {-1, result};
                    }
                }

                // WHI("Result: %i, s2 length: %li\n", result, s2.length());
                if (result == s2.size() && s1.length() == s2.length()) {
                    // BWHI("substrcmp MATCH detected: result: %i, s2.length: %li\n", result, s2.length());
                    return {0, result};
                }

                std::string sub = s1.substr(0, i);
                if (sub == s2) {
                    // BGRE("Over sub detected\n");
                    return {1, result};
                }

            } else if (c1 < c2) {
                return {-1, result};      
            } else if (c1 > c2) {
                return {1, result};
            }
        }
    }
    return {-1, result};
}

}

std::string parseClassName(std::string bytes) {
    int sep = 0;
    bool term = false;
    std::string sub;
    for (auto c : bytes) {
        if (c == ':') { sep++; continue; }
        if (c == '(') { term = true; continue; }
        if (sep == 2 && !term && c != '~') sub += c;
    }
    if (sub == "") sub = "undefined";
    // BYEL("Class name: %s\n", sub.c_str());
    return sub;
}

Trace* GLOBAL_TRACE = new Trace;

void exit_func() { TRACESCAN; }