#include "server/iris.h"
#include "util/file_system.hpp"

#include <unordered_map>

LEXES scanzz(std::string substr, std::regex rgx) {
    LEXES result;
    std::regex_iterator<std::string::iterator> it(substr.begin(), substr.end(), rgx);
    std::regex_iterator<std::string::iterator> end;
    for (; it != end; ++it) {
        LEX l({it->str(), it->position()});
        result.push_back(l);
    }
    return result;
}

// bool validate(std::string substr, std::regex rgx) {
//     std::regex_iterator<std::string::iterator> it(substr.begin(), substr.end(), rgx);
//     std::regex_iterator<std::string::iterator> end;
//     if (std::distance(it, end) != 0) {
//         return true;
//     }
//     return false;
// }

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
    void inject(std::string symbols, std::string& file) {}

    void replace(std::string& str, std::string sub, std::string rep) {
        std::string::size_type p = str.find(sub);
        if (p != std::string::npos) {
            str.replace(p, sub.size(), rep);
        }
    }

    std::string interpret(Router* router, std::string file) {
        std::string file_contents = JFS::read(file.c_str());
        size_t p = file.find_last_of('/');
        printf("last of: %li\n", p);
        std::string dir = file.substr(0, p + 1);
        std::string file_name = file.substr(p + 1, file.size());
        std::string cacheDir = dir + "cache";
        clearCache(cacheDir);
        bool val = checkMeta(cacheDir, file);
        if (val) {
            BYEL("Meta check: %i\n", (int)val);
            std::string cacheCheck = checkCache(cacheDir, file_name);
            if (cacheCheck != "undefined") {
                BGRE("Cache match found\n");
                JFS::modifiedAt(file.c_str());
                return cacheCheck;
            }
        }
        printf("dir: %s\n", dir.c_str());
        std::vector<std::string> files = JFS::getDir(dir);
        JFS::mkDir(dir + "/cache");
        LEXES ls = scanzz(file_contents, std::regex("\\$\\[(idoc)\\]\\$"));
        LEXES partials = scanzz(file_contents, std::regex("\\$\\[(ipart)\\]\\{.+\\}(.+\\$|\\$)"));
        LEXES forms = scanzz(file_contents, std::regex("\\$\\[iform=.+\\]\\{.+\\}(.+\\$|\\$)"));
        LEXES links = scanzz(file_contents, std::regex("\\$\\[ilink(=.+)?\\]\\{.+\\}(.+\\$|\\$)"));
        if (ls.size() > 0) {
            BBLU("Iris activated\n");
            for (auto lex : ls) {
                printf("iris doc found: %s - [%i,%li]\n", lex.first.c_str(), lex.second, lex.second + lex.first.size() - 1);
                replace(file_contents, lex.first, std::string(""));
            }
        } else {
            BRED("idoc not detected\n");
        }

        if (links.size() > 0) {
            BBLU("Iris link detected\n");
            for (auto lex : links) {
                std::string link = parseLink(router, lex);
                replace(file_contents, lex.first, link);
            }
        } else {
            BRED("ilink not detected\n");
        }

        if (forms.size() != 0) {
            BBLU("Form found\n");
            for (auto lex : forms) {
                printf("iris forms: %s - [%i,%li]\n", lex.first.c_str(), lex.second, lex.second + lex.first.size() - 1);
            }
        }

        LEXES temps = scanzz(file_contents, std::regex("\\$\\[itemp=(.+)\\](\\(.+\\))?\\{[\\s\\S]+\\}(.+\\$|\\$)"));
        if (temps.size() != 0) {
            BGRE("Temp found\n");
            for (auto lex : temps) {
                printf("iris templates: %s - [%i,%li]\n", lex.first.c_str(), lex.second, lex.second + lex.first.size() - 1);
                std::string bytes = parseTemp(dir, lex);
                std::string pre = file_contents.substr(0, lex.second);
                std::string post = file_contents.substr(lex.second + lex.first.size(), file_contents.size() - lex.second - lex.first.size());
                file_contents = pre + bytes + post;
            }
        } else {
            BRED("No temp found\n");
        }

        if (partials.size() > 0) {
            BBLU("Partials detected\n");
            std::string partial = "";
            for (auto lex : partials) {
                printf("partial found: %s - [%i,%li]\n", lex.first.c_str(), lex.second, lex.second + lex.first.size() - 1);
                bool append = false;
                for (auto c : lex.first) {
                    if (c == '{') { append = true; continue; } 
                    if (c == '}') { append = false; continue; }
                    if (append) {
                        partial += c;
                    }
                }
                BGRE("Partial path is: %s\n", partial.c_str());
                Methods method_calls = parseMethods(lex.first);
                bool partial_match = false;
                std::string partial_path;
                std::string partial_contents;
                if (partial.find("@") != std::string::npos) {
                    std::string packName;
                    std::string packPath;
                    bool pack = false;
                    bool packPathGate = false;
                    for (auto c : partial) {
                        if (c == '@') { pack = true; continue; }
                        if (c == '.') { pack = false; packPathGate = true; continue; }
                        if (pack) {
                            packName += c;
                        }

                        if (packPathGate) {
                            packPath += c;
                        }

                        std::string packageDir = "./public/frontend/" + packName + "/";
                        std::vector<std::string> packageFiles = JFS::getDir(packageDir);

                        for (auto f : packageFiles) {
                            if (f == "_" + packPath) {
                                partial_match = true;
                                partial_path = packageDir + "_" + packPath;
                                partial_contents = JFS::read(partial_path.c_str());
                            }
                        }
                    }
                } else {
                    for (auto pfile : files) {
                        if (pfile == "_" + partial) {
                            partial_match = true;
                            partial_path = dir + "_" + partial;
                            partial_contents = JFS::read(partial_path.c_str());
                        }
                    }
                }
                if (partial_match) {
                    BGRE("PARTIAL MATCH FOUND\n");
                    execMethods(method_calls, dir, partial, partial_contents, IS_KV);
                    // BMAG("Partial contents: %s\n", partial_contents.c_str());
                    replace(file_contents, lex.first, partial_contents);
                } else {
                    BRED("PARTIAL MATCH NOT FOUND - need error reporting\n");
                    replace(file_contents, lex.first, std::string(""));                
                }
                partial = "";
                IS_KV = false;
            }
        }
        cache(cacheDir, file_name, file_contents);
        createMeta(cacheDir, file);
        BGRE("Finished interpretting...\n");
        BGRE("%s\n", file_contents.c_str());
        return file_contents;
    }

    std::string parseLink(Router* router, LEX lex) {
        std::string chunk = lex.first;
        bool linkGate = false;
        bool labelGate = false;
        std::string linkPath;
        std::string label;
        for (auto c : chunk) {
            if (c == '{') { linkGate = true; continue; }
            if (c == ']') { labelGate = false; continue; }
            if (c == '}') { linkGate = false; continue; }
            if (c == '=') { labelGate = true; continue; }
            if (linkGate) {
                linkPath += c;
            }
            if (labelGate) {
                label += c;
            }
        }
        if (linkPath != "") {
            if (router->router()->contains(linkPath)) {
                std::string ipath = router->router()->ipath(linkPath);
                if (label == "") {
                    label = ipath;
                }
                std::string linkElement = "<a class=\"link\" href=\""+ipath+"\">" + label + "</a>";
                return linkElement;
            }
        }
        return "undefined - should throw iris error";
    }

    std::string parseForm(LEX lex) {
        return "TODO";
    }

    std::string parseTemp(std::string dir, LEX lex) {
        std::string temp = lex.first;
        std::string name;
        std::string packageName;
        std::string bodyStr;
        bool bracket = false;
        bool tempName = false;
        bool body = false;
        bool bracketBody = false;
        bool parameters = false;
        bool pack = false;
        std::string paramStr;
        for (auto c : temp) {
            if (c == '[') { bracket = true; continue; }
            if (c == '=') { tempName = true; continue; }
            if (c == ']') { bracket = false; tempName = false; continue; }
            if (c == '{') { body = true; continue; }
            if (c == '}') { body = false; bracketBody = true; continue; }
            if (c == '(') { parameters = true; continue; }
            if (c == ')') { parameters = false; continue; }
            if (c == '@') { pack = true; continue; }
            if (c == '.' && pack) { pack = false; continue; }
            if (bracket && tempName && !pack) {
                name += c;
            }
            if (pack) {
                packageName += c;
            }
            if (body) {
                bodyStr += c;
            }
            if (parameters && !bracketBody) {
                paramStr += c;
            }
        }
        bool capture = false;
        Methods methods = parseMethods(lex.first);
        int iter = 1;
        for (auto m : methods) {
            if (m.first == "many") {
                iter = std::stoi(m.second[0]);
            }
        }
        BBLU("Iterate: %i\n", iter);
        Deltas ds;
        if (paramStr != "") {
            BYEL("Param str detected: %s\n", paramStr.c_str());
            if (paramStr.find(": ") != std::string::npos) {
                size_t p = paramStr.find(": ");
                std::string captured = paramStr.substr(0, p);
                std::string captureVal = paramStr.substr(p + 2, paramStr.size());
                BYEL("Capture: %s\n", captured.c_str());
                BYEL("Capture Val: %s\n", captureVal.c_str());
                ds = ormScript(captureVal);
                capture = true;
            }
        }
        int i = 0;
        std::string result;
        while (i < iter) {
            std::vector<std::string> toks = prizm::tokenize(bodyStr, ',');
            std::vector<std::pair<std::string, std::string>> kvs;
            for (auto t : toks) {
                prizm::erase(t, '\t');
                prizm::erase(t, '\n');
                prizm::erase(t, '\r');
                prizm::trim(t);
                BWHI("tok: %s\n", t.c_str());
                std::string key = "undefined";
                std::string val = "undefined";
                if (size_t p = t.find(": ") != std::string::npos) {
                    p = t.find(": ");
                    BMAG("idx: %li\n", p);
                    BMAG("t: %s\n", t.c_str());
                    key = t.substr(0, p);
                    prizm::erase(key, ' ');
                    val = t.substr(p+2, t.size());
                    BBLU("Key: %s, Val: %s\n", key.c_str(), val.c_str());
                    if (val.at(0) == '"') { val.erase(val.begin()); }
                    if (val.at(val.size() - 1) == '"') { val.pop_back(); }
                    if (capture) {
                        BYEL("Need to parse val\n");
                        size_t ent_p = val.find(".");
                        std::string ent = val.substr(0, ent_p);
                        std::string cent = prizm::lowercase(ent);
                        std::string entVal = val.substr(ent_p + 1, val.size());
                        BYEL("Entity: %s, attr: %s\n", cent.c_str(), entVal.c_str());
                        IrisAttrs args = ds[cent][0];
                        BBLU("Got here: %li\n", entVal.length());
                        for (auto a : args) {
                            printf("%s: %s\n", a.first.c_str(), a.second);
                            printf("%li\n", strlen(a.second));
                            if (entVal == std::string(a.first)) {
                                BGRE("MATCH MF\n");
                            }
                        }
                        BBLU("Got here 2: %s\n", entVal.c_str());
                        std::string s(args[entVal]);
                        BYEL("WTH\n");
                        BMAG("Val: %s\n", s.c_str())

                        val = s;
                    }
                }
                BMAG("Template key: %s, val: %s\n", key.c_str(), val.c_str());
                std::pair<std::string, std::string> kv{key, val};
                kvs.push_back(kv);
            }

            // for @packages
            std::string path;
            if (packageName != "") {
                dir = "./public/frontend/" + packageName + "/";
            }
            BYEL("Temp name: %s\n", name.c_str());
            path = dir + "@" + name;
            BYEL("Path is: %s\n", path.c_str());

            // reads template file
            std::string bytes = JFS::read(path.c_str());
            LEXES yields = scanzz(bytes, std::regex("\\$\\[iyield\\]\\{.+\\}(.+\\$|\\$)"));
            std::vector<std::pair<std::string, LEX>> yieldTargets;
            if (yields.size() != 0) {
                BBLU("Yield found\n");
                for (auto lex : yields) {
                    printf("iris yields: %s - [%i,%li]\n", lex.first.c_str(), lex.second, lex.second + lex.first.size() - 1);
                    bool start = false;
                    std::string yield;
                    for (auto c : lex.first) {
                        if (c == '{') { start = true; continue; }
                        if (c == '}') { start = false; break; }
                        if (start) {
                            yield += c;
                        }
                    }
                    BYEL("Yield: %s\n", yield.c_str());
                    yieldTargets.push_back({yield, lex});
                }
            }
            std::string newStr;
            size_t cptr = 0;
            for (auto kv : kvs) {
                std::string key = kv.first;
                std::string val = kv.second;
                BWHI("Key: %s, Val: %s\n", key.c_str(), val.c_str());
                for (auto y : yieldTargets) {
                    if (y.first == key) {
                        std::string sub = bytes.substr(cptr, y.second.second - cptr);
                        // BGRE("Sub: %s\n", sub.c_str());
                        // BYEL("Cptr init: %li\n", cptr);
                        // BMAG("Lex idx: %i\n", y.second.second);
                        sub += val;
                        newStr += sub;
                        cptr = y.second.second + y.second.first.length();
                        // BYEL("Cptr: %li\n", cptr);
                        // BBLU("NewStr: %s\n", newStr.c_str());
                        // BMAG("Bytes: %s\n", bytes.c_str());
                        // BGRE("Byte substr test: %s\n", bytes.substr(72, 115 - 72).c_str());
                    }
                }
            }
            newStr += bytes.substr(cptr, bytes.size() - cptr);
            result += newStr + "\n";
            i++;
        }
        BCYA("New Str: %s\n", result.c_str());
        return result;
    }

    typedef std::vector<std::pair<std::string, std::vector<std::string>>> Methods;
    Methods parseMethods(std::string lex) {
        // std::string methods = lex.first.substr(p2 + 1, lex.first.size());
        size_t p2 = lex.find_last_of("}");
        std::string methods = lex.substr(p2 + 1, lex.size());
        BYEL("Methods: %s\n", methods.c_str());
        std::vector<std::pair<std::string, std::vector<std::string>>> method_calls = {};
        IS_KV = false;
        if (methods[0] == '$') {
            YEL("No methods detected\n");
        } else if (methods[0] == '.') {
            GRE("Method detected\n");
            methods = methods.substr(0, methods.size() - 1);
            bool name = false;
            bool params = false;
            bool literal = false;
            std::string name_str = "";
            std::string param_str = "";
            for (auto c : methods) {
                if (c == '.' && !literal) { 
                    if (name_str != "") {
                        BYEL("Method name: %s\n", name_str.c_str());
                        BYEL("Param name: %s\n", param_str.c_str());
                        std::vector<std::string> toks = prizm::tokenize(param_str, ',');
                        std::pair<std::string, std::vector<std::string>> pair = {name_str, toks};
                        method_calls.push_back(pair);
                        name_str = "";
                        param_str = "";
                    }
                    name = true; continue;
                }
                if (c == '"') { literal = !literal; }
                if (c == '(') { name = false; params = true; continue; }
                if (c == ')') { name = false; params = false; continue; }
                if (c == ':' && !literal) { IS_KV = true; }
                if (name) {
                    name_str += c;
                }
                if (params) {
                    param_str += c;
                }
            }
            if (name_str != "") {
                BYEL("Method name: %s\n", name_str.c_str());
                BYEL("Param name: %s\n", param_str.c_str());
                std::vector<std::string> toks = prizm::tokenize(param_str, ',');
                std::pair<std::string, std::vector<std::string>> pair = {name_str, toks};
                method_calls.push_back(pair);
                name_str = "";
                param_str = "";
            }
        } else {
            BRED("Invalid operator detected on iris syntax\n");
        }
        return method_calls;
    }

    LEXES scanzz(std::string substr, std::regex rgx) {
        LEXES result;
        std::regex_iterator<std::string::iterator> it(substr.begin(), substr.end(), rgx);
        std::regex_iterator<std::string::iterator> end;
        for (; it != end; ++it) {
            LEX l({it->str(), it->position()});
            result.push_back(l);
        }
        return result;
    }

    std::string checkCache(std::string dir, std::string name) {
        std::vector<std::string> files = JFS::getDir(dir);
        std::string cachePath = name + ".cache";
        BYEL("Checking cache for %s\n", cachePath.c_str());
        for (auto f : files) {
            if (f == cachePath) {
                BGRE("Cache match\n");
                std::string bytes = JFS::read(cachePath.c_str());
                return bytes;
            }
        }
        return "undefined";
    }

    void cache(std::string dir, std::string name, std::string contents) {
        std::string cachePath = dir + "/" + name + ".cache";
        JFS::write(cachePath.c_str(), contents, true);
    }

    void clearCache(std::string dir) {
        std::vector<std::string> files = JFS::getDir(dir);
        for (auto f : files) {
            std::string path = dir + "/" + f;
            if (remove(path.c_str()) != 0) {
                BRED("Error removing file: %s\n", path.c_str());
            }
        }
    }

    void createMeta(std::string dir, std::string path) {
        std::string imeta = dir + "/index.imeta";
        std::string bytes = JFS::read(imeta.c_str());
        std::string offset = path + ": ";
        if (bytes.find(offset) != std::string::npos) {
            char* p = strstr(&bytes[0], offset.c_str());
            char* p2 = strstr(p, "\n");
            p += strlen(offset.c_str());
            int idx1 = (int)(p - &bytes[0]);
            int idx2 = (int)(p2 - &bytes[0]);
            BYEL("Indices: %i : %i\n", idx1, idx2);
            BYEL("bytes: %s\n", bytes.c_str());
            long mod = JFS::modifiedAt(path.c_str());
            std::string s = std::to_string(mod);
            strncpy(&bytes[0] + idx1, s.c_str(), idx2 - idx1);
            BBLU("bytes: %s\n", bytes.c_str());
            JFS::write(imeta.c_str(), bytes.c_str(), true);
        } else {
            BRED("Meta not found\n");
            long mod = JFS::modifiedAt(path.c_str());
            offset += std::to_string(mod);
            offset += "\n";
            JFS::write(imeta.c_str(), offset);
        }
    }

    bool checkMeta(std::string dir, std::string path) {
        std::string imeta = dir + "/index.imeta";
        std::string offset = path + ": ";
        CYA("Offset is: %s\n", offset.c_str());
        std::string bytes = JFS::read(imeta.c_str());
        if (bytes.find(offset) != std::string::npos) {
            char* p;
            char tmp[bytes.size()];
            strncpy(tmp, bytes.c_str(), bytes.size());
            p = strstr(tmp, offset.c_str());
            p += strlen(offset.c_str());
            if (p == NULL) return false;
            char* p2 = strstr(p, "\n");
            if (p2 == NULL) return false;
            int idx1 = (int)(p - tmp);
            int idx2 = (int)(p2 - tmp);
            int len = idx2 - idx1;
            char buffer[p2 - p];
            strncpy(buffer, p, p2 - p);
            buffer[p2 - p] = 0;
            printf("Buffer len is: %i\n", len);
            printf("idx1: %i, idx2: %i\n", idx1, idx2);
            printf("Buffered timestamp is: %s\n", buffer);
            long mod = JFS::modifiedAt(path.c_str());
            std::string s = std::to_string(mod);
            if (strcmp(buffer, s.c_str()) != 0) {
                BRED("No match on strings: read: %s (%li), actual: %s (%li)\n", buffer, sizeof(buffer), s.c_str(), s.size());
                return false;
            } else {
                BGRE("Meta timestamps match!\n");
            }
            return true;
        }
        BRED("Meta offset not found\n");
        return false;
    }

    Deltas ormScript(std::string script) {
        BYEL("ORM SCRIPT IS: %s\n", script.c_str());
        // Args args = Args{ {"fname", "jose"}, {"email", "all@day.com"}, {"lname", "cuervo"}, {"username", "tequila"}};
        return Deltas{{"user", {{{"fname", "jose"}, {"email", "all@day.com"}, {"lname", "cuervo"}, {"username", "tequila"}}}}};
    }

    void execMethods(std::vector<std::pair<std::string, std::vector<std::string>>> methods, std::string dir, std::string insName, std::string insVal, bool kv) {
        BCYA("\nExecuting methods...\n---------------------------------\n");
        for (auto method : methods) {
            std::string method_name = method.first;
            std::vector<std::string> method_args = method.second;
            BMAG("Method: %s\n", method.first.c_str());
            if (kv) {
                for (auto arg : method_args) {
                    if (arg.find(": ") != std::string::npos) {
                        size_t p = arg.find(": ");
                        std::string arg_name = arg.substr(0, p);
                        std::string arg_val = arg.substr(p + 2, arg.size());
                        BYEL("KV arg - name: %s, val: %s\n", arg_name.c_str(), arg_val.c_str());
                    } else {
                        BRED("KV arg missing\n");
                    }
                }
            } else {
                for (auto arg : method_args) {
                    BYEL("Normal arg - val: %s\n", arg.c_str());
                } 
            }

            if (method_name == "cache") {
                std::string cacheDir = dir + "cache";
                BBLU("Cache dir: %s\n", cacheDir.c_str());
                if (JFS::dirExists(cacheDir)) {
                    std::string cachePath = cacheDir + "/" + insName + ".cache";
                    JFS::write(cachePath.c_str(), insVal, true);
                } else {
                    BRED("Dir does not exist: %s\n", cacheDir.c_str());
                }
            }

        }
    }

}
