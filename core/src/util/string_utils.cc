#include "util/string_utils.h"

std::string join(std::vector<std::string> vec, std::string delim, int start, int end) {
    if (start == -1 && end == -1) {
        start = 0;
        end = vec.size() - 1;
    } else if (start == -1 && end != -1) {
        start = 0;
    } else if (start != -1 && end == -1) {
        end = vec.size() - 1;
    }

    if (start < 0 || start > vec.size() - 1) {
        BRED("Invalid start index for join");
        return "";
    }

    if (end < 0 || end > vec.size() - 1) {
        BRED("Invalid end index for join");
        return "";
    }

    std::string result = "";
    for (int i = start; i <= end; i++) {
        // if (typeid(T) == typeid(std::string)) {
        result += vec[i];
        // } else {
        //     result += std::to_string(i);
        // }
    }

    return result;
}

std::string serialize(const std::string& name, const std::vector<std::string>& vec) {
    std::string response = "{\"" + name + "\": ["; 
    for (int i = 0; i < vec.size(); i++) {
        response += vec[i];
    }
}

std::string Jericho::reverse(std::string s) {
    std::string str = s;
    int n = str.length();
    for (int i = 0; i < n / 2; i++) {
        std::swap(str[i], str[n - i - 1]);
    }
    return str;
}

std::vector<std::string> Jericho::reverse(std::vector<std::string> s) {
    // bold(); yellow();
    // printf("reversing\n");
    // for (auto w : s) {
    //     printf("%s\n", w.c_str());
    // }
    int n = s.size();
    for (int i = 0; i < n / 2; i++) {
        std::swap(s[i], s[n - i - 1]);
    }
    // for (auto w : s) {
    //     printf("%s\n", w.c_str());
    // }
    return s;
    // clearcolor();
}
 
std::string Jericho::ltrim(const std::string &s) {
    return std::regex_replace(s, std::regex("^\\s+"), std::string(""));
}
 
std::string Jericho::rtrim(const std::string &s) {
    return std::regex_replace(s, std::regex("\\s+$"), std::string(""));
}
 
std::string Jericho::trim(const std::string &s) {
    return ltrim(rtrim(s));
}

void Jericho::ltrim_force(std::string& s) {
    s = std::regex_replace(s, std::regex("\\s+$"), std::string(""));
}

void Jericho::rtrim_force(std::string& s) {
    s = std::regex_replace(s, std::regex("\\s+$"), std::string(""));
}

void Jericho::trim_force(std::string& s) {
    ltrim(rtrim(s));
}

void Jericho::erase(std::string& str, char toRemove) {
    str.erase(std::remove(str.begin(), str.end(), toRemove), str.end());
}

void Jericho::trim_tokens(std::vector<std::string>& toks) {
    for (int i = 0; i < toks.size(); i++) {
        toks.at(i) = trim(toks.at(i));
    }
}

void Jericho::dump_tokens(std::vector<std::string> toks) {
    for (auto t : toks) {
        std::cout << "-> " << t << std::endl;
    }
}

std::vector<std::string> Jericho::tokenize(std::string in, char delim) {
    std::istringstream ss(in);
    std::string word;
    std::vector<std::string> words;
    while (std::getline(ss, word, delim)) {
        if (word != "") {
            words.push_back(word);
        }
    }
    return words;
}

std::vector<std::string> Jericho::tokenize(std::string in, std::string delim) {
    size_t last = 0;
    size_t next = 0;
    std::vector<std::string> result = {};
    while ((next = in.find(delim, last)) != std::string::npos) {
        if (in.substr(last, next-last) != "") {
            result.push_back(in.substr(last, next-last));
        }
        last = next + delim.size();
    }
    if (in.substr(last, next-last) != "") {
        result.push_back(in.substr(last));
    }
    return result;
}

LEXES Jericho::lex_locs(std::string substr, std::regex rgx) {
    LEXES result;
    std::regex_iterator<std::string::iterator> it(substr.begin(), substr.end(), rgx);
    std::regex_iterator<std::string::iterator> end;
    for (; it != end; ++it) {
        LEX l({it->str(), it->position()});
        result.push_back(l);
    }
    return result;
}

bool Jericho::lex_contains(std::string substr, std::regex rgx) {
    LEXES result;
    std::regex_iterator<std::string::iterator> it(substr.begin(), substr.end(), rgx);
    std::regex_iterator<std::string::iterator> end;
    for (; it != end; ++it) {
        LEX l({it->str(), it->position()});
        result.push_back(l);
    }
    return (result.size() > 0);
}

bool Jericho::lex_is(std::string substr, std::regex rgx) {
    std::regex_iterator<std::string::iterator> it(substr.begin(), substr.end(), rgx);
    std::regex_iterator<std::string::iterator> end;
    if (std::distance(it, end) > 0) {
        return true;
    }
    return false;
}

std::string Jericho::replace(std::string s, char a, char b) {
    std::string result = s;
    std::replace( result.begin(), result.end(), a, b); // replace all 'x' to 'y'
    return result;
}