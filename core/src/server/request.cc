#include "server/request.h"
#include "util/encoding.h"

using namespace Jericho;

std::unordered_map<std::string, ContentType> content_type_map = {
    {"application/octet-stream", CONTENT_OCTET}
};

std::unordered_map<std::string, EncodingType> encoding_map = {
    {"base64", ENCODING_BASE64}
};

void Request::eval() {
    BCYA("Evaling request!\n");
    std::string bytes(client->request);
    // BYEL("Request::eval example:\n");
    // printf("%s\n", bytes.c_str());

    std::string::size_type p = bytes.find("\r\n\r\n");
    if (p != std::string::npos) {
        // BBLU("Term location: %li :: %li\n", p, bytes.size());
    } else {
        BRED("Request::eval: Invalid headers (\\r\\n\\r\\n not found)\n");
        this->valid = false;
    }

    std::string headersStr;
    std::string content;

    if (p != bytes.size() - 4) {
        size_t last = 0;
        size_t next = 0;
        std::vector<std::string> result;
        std::string delim = "\r\n\r\n";
        while ((next = bytes.find(delim, last)) != std::string::npos) {
            if (bytes.substr(last, next-last) != "") {
                result.push_back(bytes.substr(last, next-last));
            }
            last = next + delim.size();
        }
        if (bytes.substr(last, next-last) != "") {
            result.push_back(bytes.substr(last));
        }
        if (result.size() == 2) {
            BWHI("Headers:\n");
            printf("%s\n", result[0].c_str());
            headersStr = result[0];
            BWHI("Content:\n");
            printf("%s\n", result[1].c_str());
            this->content = result[1];
        } else if (result.size() == 1) {
            BWHI("Request::eval: Headers only!\n");
        }
    } else {
        headersStr = bytes;
    }
    
    parseHeaders(headersStr);        

    BGRE("CONTENT: %s\n", this->content.c_str());
}

void Request::parseHeaders(std::string headerStr) {
    std::istringstream ss(headerStr);
    std::string word;
    std::vector<std::string> words;
    bool first = true;
    std::string protoHead;
    std::unordered_map<std::string, std::string> args2;
    std::unordered_map<std::string, std::string> callbackMap;
    while (std::getline(ss, word, '\n')) {
        if (word != "") {
            std::string::size_type sep = word.find(": ");
            if (sep != std::string::npos && !first) {
                std::string key = word.substr(0, sep);
                std::string val = word.substr(sep+2, word.size() - sep+2);
                prizm::erase(val, '\r');
                if (key == "Host") {
                    host = val;
                } else if (key.find("Callback-") != std::string::npos) {
                    callbackMap[key] = val;
                }
                if (key == "Content-Encoding") {
                    if (prizm::contains_key(encoding_map, val)) {
                        this->encoding = encoding_map[val];
                    }
                }
                if (key == "Content-Type") {
                    if (prizm::contains_key(content_type_map, val)) {
                        this->type = content_type_map[val];
                    }
                }
                if (key == "Content-Size") {
                    // need safe std::stoi
                    bool is_digit = true;
                    for (auto& c : val) {
                        if (!std::isdigit(c)) {
                            is_digit = false;
                        }
                    }
                    if (is_digit) {
                        this->size = std::stoi(val);
                    } else {
                        this->size = 0;
                    }
                }
                args2[key] = val;
            } else if (first) {
                protoHead = word;
                first = !first;
            }
            words.push_back(word);
        }
    }
    if (callbackMap.size() != 0) {
        callback = new Callback(callbackMap);
    }

    int sepGate = 0;
    std::string method2;
    std::string path2;
    std::string protocol2;
    for (auto c : protoHead) {
        if (c == ' ') { sepGate++; continue; }
        if (c == '\t') { continue; }
        if (c == '\r') { continue; }
        if (sepGate == 0) { method2 += c; }
        else if (sepGate == 1) { path2 += c; }
        else if (sepGate == 2) { protocol2 += c; }
    }
    this->method = method2;
    this->path = path2;
    this->protocol = protocol2;
    this->headers = args2;

    size_t pathp = this->path.find("?");
    if (pathp != std::string::npos) {
        std::string npath = this->path.substr(pathp+1, this->path.size());
        this->path = this->path.substr(0, pathp);
        std::vector<std::string> toks = prizm::tokenize(npath, '&');
        for (auto t : toks) {
            std::string::size_type p = t.find("=");
            if (p != std::string::npos) {
                std::string key = t.substr(0, p);
                std::string val = t.substr(p+1, t.size());
                args[key] = val;
            }
        }
    }

    if (this->method == "POST" && this->content != "") {
        if (this->type == CONTENT_OCTET) {
        char* p;
        if ((p = strstr(client->request, "\r\n\r\n")) != NULL) {
            std::string cont = std::string(p+4, jcrypt::base64::size_num(this->size));
            std::string s;
            bool has_null = false;
            if (this->encoding == ENCODING_BASE64) {
                BMAG("CONTENT: %s\n", cont.c_str());
                for (auto& c : cont) {
                    if (c == '\0') {
                        BRED("NULL CHAR FOUND!\n");
                        has_null = true;
                    }
                }
                if (has_null) {
                    cont.pop_back();
                }
                s = jcrypt::base64::decode_url(cont);
                BRED("ENCODING IS BASE64: %li\n", s.size());
                BYEL("DECODED: %s\n", s.c_str());
            }
            BRED("CONTENT SIZE IS: %i\n", this->size);
            for (auto& c : s) {
                if (c == '\0') {
                    printf("\\0");
                } else {
                    printf("%c", c);
                }
            }
            printf("\n");
            this->content = std::string(s.data(), s.size());
            BMAG("THIS CONTENT IS: %s\n", this->content.c_str());
        } 
        } else {
            if (this->content.find("&") != std::string::npos) {
                std::vector<std::string> toks = prizm::tokenize(this->content, '&');
                for (auto t : toks) {
                    std::string::size_type p = t.find("=");
                    if (p != std::string::npos) {
                        std::string key = t.substr(0, p);
                        std::string val = t.substr(p+1, t.size());
                        args[key] = val;
                    }
                }
            }
        }
    }
}

bool isHTTP(std::string request) {
    std::vector<std::string> cpath = tokenize(request, ' ');
    if (cpath.size() >= 3) {
        if (cpath[0] == "HTTP/1.1" || cpath[0] == "HTTP/2.0") {
            return true;
        }
    } else {
        return false;
    }
    return false;
}

int parse_request(Client* client, Request* request) {
    // CYA("request in parse: %s\n", client->request);
    // BGRE("........REQUEST LENGTH: %li\n", sizeof(client->request));
    int g = strncmp("GET /", client->request, 5);
    int p = strncmp("POST /", client->request, 6);
    std::string cxxreq = std::string(client->request);
    std::string cpath = tokenize(cxxreq, ' ')[1];
    std::string args = "";
    if (cpath.find("?") != std::string::npos) {
        std::vector<std::string> result = tokenize(cpath, "?");
        cpath = result[0];
        args = result[1];
    }
    if (g && p) {
        return 0;
    } else {
        if (!g) {
            char* path = client->request + 4; // removes "GET "
            char* end_path = strstr(path, " "); // finds first occurence of " "
            char* p = strstr(client->request, "\r\n\r\n");
            int idx = (int)(path - client->request);
            int idx2 = (int)(end_path - client->request);
            int max_path_length = 1000;
            if (idx2 - idx > max_path_length - 1) {
                printf("Path is too long\n");
                return 0;
            } 
            char subbuf[idx2 - idx + 1];
            strncpy(subbuf, path, (size_t)idx2 - idx);
            char* headers = strstr(end_path, "\r\n");
            if (!end_path) {
                return 0;
            } else {
                request->method = "get";
                request->path = cpath;
                std::string heading = std::string(headers + 2);
                std::vector<std::string> h = tokenize(heading, "\r\n\r\n");
                request->request = std::string(headers + 2);
                std::vector<std::string> headers = tokenize(h[0], "\r\n");
                for (auto h : headers) {
                    std::vector<std::string> kv = tokenize(h, ": ");
                    if (kv.size() > 1) {
                        request->headers[kv[0]] = kv[1]; 
                    }
                }
                if (h.size() > 1) {
                    std::string content = h[1];
                    BWHI("SIZE OF CONTENT: %li\n", h[1].size());
                    request->content = content;
                }
                std::unordered_map<std::string, std::string> map = {};
                if (args != "") {
                    std::vector<std::string> argu = tokenize(args, "&");
                    for (auto a : argu) {
                        std::vector<std::string> kv = tokenize(a, "=");
                        if (kv.size() == 2) {
                            map[kv[0]] = kv[1];
                        }
                    }
                }
                if (prizm::contains_key(request->headers, std::string("Content-Type")) && prizm::contains_key(request->headers, std::string("Content-Length"))) {
                    std::string ctype = request->headers["Content-Type"];
                    std::string clen = request->headers["Content-Length"];
                    map["Content-Type"] = ctype;
                    map["Content-Length"] = clen;
                    if (ctype == "binary") {
                        BBLU("BINARY CONTENT DETECTED\n");
                        long sz = std::stol(clen);
                        BYEL("SZ IS: %li\n", sz);
                        BYEL("RECEIVED BYTES ARE: %i\n", client->received);
                        char buf[sz];
                        memcpy(buf, p + 4, sz);
                        int i = 0;
                        Jericho::FileSystem::writeBinary("./request.txt", client->request);
                        std::string bytes = Jericho::FileSystem::readBinary("./py/scripts/torch.pt");
                        Jericho::FileSystem::writeBinary("./binary.txt", bytes.data());
                        int err_ct = 0;
                        BYEL("COMPARE SIZES: %li == %li\n", sz, bytes.size());
                        // for (int i = 0; i < bytes.size(); i++) {
                        //     if (bytes[i] != buf[i]) {
                        //         err_ct++;
                        //     }
                        // }
                        BRED("ERROR CT IS: %i\n", err_ct);
                        printf("\n");
                        std::vector<char> vec;
                        vec.insert(vec.end(), buf, buf + sizeof(buf));
                        BBLU("===================\n\n\n");
                        std::string s(vec.begin(), vec.end());
                        // std::cout << s << std::endl;
                        BBLU("CONTENT SIZE IS: %li\n", s.size());
                        BBLU("CONTENT VEC SIZE IS: %li\n", vec.size());
                        BBLU("CONTENT VEC SIZE IS: %li\n", sizeof(buf));
                        BBLU("===================\n\n\n");
                        request->content = s;
                    } else {
                        long sz = std::stol(clen);
                        char buf[sz];
                        strncpy(buf, p + 4, sz);
                        buf[sz] = 0;
                        std::string content(buf);
                        BWHI("SIZE OF CONTENT: %li\n", content.size());
                        request->content = content;
                    }
                }
                map["content"] = request->content;
                if (prizm::contains_key(request->headers, std::string("Host"))) {
                    map["Host"] = request->headers["Host"];
                    client->url = request->headers["Host"];
                }
                request->args = map;
                for (auto m : map) {
                    BGRE("%s: %s\n",m.first.c_str(), m.second.c_str());
                }
                *end_path = 0; // zero out char
                return 1; // static file serving
            }
        } else if (!p) {
            char* path = client->request + 5; // removes "GET "
            char* end_path = strstr(path, " "); // finds first occurence of " "
            char* ptr = strstr(client->request, "\r\n\r\n");
            int idx = (int)(path - client->request);
            int idx2 = (int)(end_path - client->request);
            int max_path_length = 1000;
            if (idx2 - idx > max_path_length - 1) {
                printf("Path is too long\n");
                return 0;
            } 
            char subbuf[idx2 - idx + 1];
            strncpy(subbuf, path, (size_t)idx2 - idx);
            char* headers = strstr(end_path, "\r\n");
            if (!end_path) {
                return 0;
            } else {
                request->method = "post";
                request->path = cpath;
                std::string heading = std::string(headers + 2);
                std::vector<std::string> h = tokenize(heading, "\r\n\r\n");
                request->request = std::string(headers + 2);
                std::vector<std::string> headers = tokenize(h[0], "\r\n");
                for (auto h : headers) {
                    std::vector<std::string> kv = tokenize(h, ": ");
                    if (kv.size() > 1) {
                        request->headers[kv[0]] = kv[1]; 
                    }
                }
                std::unordered_map<std::string, std::string> map = {};
                if (prizm::contains_key(request->headers, std::string("Content-Type")) && prizm::contains_key(request->headers, std::string("Content-Length"))) {
                    std::string ctype = request->headers["Content-Type"];
                    std::string clen = request->headers["Content-Length"];
                    map["Content-Type"] = ctype;
                    map["Content-Length"] = clen;
                    if (ctype == "binary") {
                        BBLU("BINARY CONTENT DETECTED\n");
                        long sz = std::stol(clen);
                        BYEL("SZ IS: %li\n", sz);
                        BYEL("RECEIVED BYTES ARE: %i\n", client->received);
                        char buf[sz];
                        memcpy(buf, ptr + 4, sz);
                        int i = 0;
                        Jericho::FileSystem::writeBinary("./request.txt", client->request);
                        std::string bytes = Jericho::FileSystem::readBinary("./py/scripts/torch.pt");
                        Jericho::FileSystem::writeBinary("./binary.txt", bytes.data());
                        int err_ct = 0;
                        BYEL("COMPARE SIZES: %li == %li\n", sz, bytes.size());
                        // for (int i = 0; i < bytes.size(); i++) {
                        //     if (bytes[i] != buf[i]) {
                        //         err_ct++;
                        //     }
                        // }
                        BRED("ERROR CT IS: %i\n", err_ct);
                        printf("\n");
                        std::vector<char> vec;
                        vec.insert(vec.end(), buf, buf + sizeof(buf));
                        BBLU("===================\n\n\n");
                        std::string s(vec.begin(), vec.end());
                        // std::cout << s << std::endl;
                        BBLU("CONTENT SIZE IS: %li\n", s.size());
                        BBLU("CONTENT VEC SIZE IS: %li\n", vec.size());
                        BBLU("CONTENT VEC SIZE IS: %li\n", sizeof(buf));
                        BBLU("===================\n\n\n");
                        request->content = s;
                    } else {
                        long sz = std::stol(clen);
                        BBLU("parse_resquest::post: SIZE %li\n", sz);
                        char buf2[sz];
                        std::string hell(ptr + 4);
                        strncpy(buf2, ptr + 4, sz);
                        buf2[sz] = '\0';
                        for (auto c : hell) {
                            std::cout << c;
                            std::cout << (char)c;
                        }
                        // int* outSz;
                        // *outSz = 5000;
                        char out[5000];
                        // if (UTF8_to_ascii(unsigned char* out, int *outlen, const unsigned char* in, int *inlen) {
                        // auto s1 = from_utf8(hell);
                        // BYEL("%s\n", s1.c_str());
                        BBLU("parse_resquest::post: original: %s\n", cxxreq.c_str());
                        BBLU("parse_resquest::post: content: %s\n", hell.c_str());
                        std::string content = buf2;
                        BBLU("parse_resquest::post: buf-size: %li\n", content.size());
                        request->content = content;
                    }
                }
                map["content"] = request->content;
                if (prizm::contains_key(request->headers, std::string("Host"))) {
                    map["Host"] = request->headers["Host"];
                    client->url = request->headers["Host"];
                }
                request->args = map;
                for (auto m : map) {
                    BGRE("%s: %s\n",m.first.c_str(), m.second.c_str());
                }
                *end_path = 0; // zero out char
                return 1; // static file serving
            }
        }
    }
    return 0;
}

bool is_distributed(std::string& path) {
    std::string temp = path;
    std::vector<std::string> args = tokenize(path, "/");
    BYEL("IS DISTRIBUTED TEST");
    for (auto a : args) {
        BYEL("ARG: %s\n", a.c_str());
        if (a == "cluster") {
            path = join(args, "/", 2);
            BYEL("PATH IS: %s\n", path.c_str());
            return true;
        }
    }
    return false;
}