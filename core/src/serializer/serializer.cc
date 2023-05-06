#include "serializer/serializer.h"
#include "server/http_status.h"

std::string http_status_to_json(std::string& http) {
    size_t p;
    std::string delim = " ";
    std::vector<std::string> toks;
    int ct = 0;
    BYEL("%s\n", http.c_str());
    while (((p = http.find(delim)) != std::string::npos) && ct < 2) {
        std::string substr = http.substr(0, p);
        toks.push_back(http.substr(0, p));
        http = http.substr(p+1, http.size() - p - 1);
        ct++;
    }
    toks.push_back(http);
    if (toks.size() != 3) {
        BRED("Serializer::http_to_json: Invalid Http status format\n");
        for (auto tok : toks) {
            YEL("Tok: %s\n", tok.c_str());
        }
        return JsonResponse::error(500, "Invalid Http status format\n");
    }

    std::string protocol = toks[0];
    std::string code_str = toks[1];
    std::string code_desc = toks[2];
    int val = 500;
    if (!parseInt(val, code_str)) {
        BRED("Serializer::http_to_json: Invalid Http status formatted string code is not of type int\n");
        RED("Code_str: %s\n", code_str.c_str());
        return JsonResponse::error(500, "Invalid Http status format\n");    
    }

    BYEL("Code_desc = %s\n", toks[2].c_str());

    picojson::object o;
    o["protocol"] = picojson::value(protocol);
    o["code"] = picojson::value(static_cast<double>(val));
    o["code-description"] = picojson::value(code_desc);

    picojson::value v(o);
    return v.serialize();
}