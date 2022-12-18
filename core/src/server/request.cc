#include "server/request.h"
#include "util/encoding.h"

using namespace Jericho;

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

bool is_valid_request(Client* client) {
    // CYA("request in parse: %s\n", client->request);
    int g = strncmp("GET /", client->request, 5);
    int p = strncmp("POST /", client->request, 6);
    if (!g || !p) {
        return true;
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
            // printf("IDX1: %i\n", idx);
            // printf("IDX2: %i\n", idx2);
            int max_path_length = 1000;
            if (idx2 - idx > max_path_length - 1) {
                printf("Path is too long\n");
                return 0;
            } 
            char subbuf[idx2 - idx + 1];
            // printf("strlen of subbuf is: %i, len of idx2 - idx is: %i\n", (int)strlen(subbuf), idx2 - idx);
            strncpy(subbuf, path, (size_t)idx2 - idx);
            // subbuf[idx2 - idx + 1] = 0;
            char* headers = strstr(end_path, "\r\n");
            if (!end_path) {
                return 0;
            } else {
                request->method = "get";
                // printf("PATH IS: %s\n", cpath.c_str());
                printf("ARGS ARE: %s\n", args.c_str());
                request->path = cpath;
                std::string heading = std::string(headers + 2);
                std::vector<std::string> h = tokenize(heading, "\r\n\r\n");
                request->request = std::string(headers + 2);
                for (auto h_ : h) {
                    BYEL("%s\n", h_.c_str());
                }
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
                        while (i < 490000) {
                            // if (buf[i] != '\0') { printf("%c", (char)buf[i]);}
                            // printf("%c", (char)client->request[i]);
                            i++;
                        }
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
            // printf("IDX1: %i\n", idx);
            // printf("IDX2: %i\n", idx2);
            int max_path_length = 1000;
            if (idx2 - idx > max_path_length - 1) {
                printf("Path is too long\n");
                return 0;
            } 
            char subbuf[idx2 - idx + 1];
            // printf("strlen of subbuf is: %i, len of idx2 - idx is: %i\n", (int)strlen(subbuf), idx2 - idx);
            strncpy(subbuf, path, (size_t)idx2 - idx);
            // subbuf[idx2 - idx + 1] = 0;
            char* headers = strstr(end_path, "\r\n");
            if (!end_path) {
                return 0;
            } else {
                request->method = "post";
                // printf("PATH IS: %s\n", cpath.c_str());
                printf("ARGS ARE: %s\n", args.c_str());
                request->path = cpath;
                std::string heading = std::string(headers + 2);
                std::vector<std::string> h = tokenize(heading, "\r\n\r\n");
                request->request = std::string(headers + 2);
                for (auto h_ : h) {
                    BYEL("%s\n", h_.c_str());
                }
                std::vector<std::string> headers = tokenize(h[0], "\r\n");
                for (auto h : headers) {
                    std::vector<std::string> kv = tokenize(h, ": ");
                    if (kv.size() > 1) {
                        request->headers[kv[0]] = kv[1]; 
                    }
                }
                std::unordered_map<std::string, std::string> map = {};
                // if (args != "") {
                //     std::vector<std::string> argu = tokenize(args, "&");
                //     for (auto a : argu) {
                //         std::vector<std::string> kv = tokenize(a, "=");
                //         if (kv.size() == 2) {
                //             map[kv[0]] = kv[1];
                //         }
                //     }
                // }
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
                        while (i < 490000) {
                            // if (buf[i] != '\0') { printf("%c", (char)buf[i]);}
                            // printf("%c", (char)client->request[i]);
                            i++;
                        }
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
                        // int* inSz;
                        // *inSz = (int)sz;
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

void print_request(Request* request) {
    // std::string ext = "undefined";
    // if (const std::string::size_type p = request->path.find_last_of(".") != std::string::npos) {
    //     ext = request->path.substr(p+1, request->path.size());
    // }
    // if (ext == "html") {
        BYEL("===============\n");
        BYEL("   REQUEST\n");
        BYEL("---------------\n");
        BYEL("method: %s\n", request->method.c_str());
        BYEL("path: %s\n", request->path.c_str());
        BYEL("content: %s\n", request->content.c_str());
        for (auto head : request->headers) {
            BYEL("header: %-32s %s\n", head.first.c_str(), head.second.c_str());
        }
        // BYEL("request: \r\n"); 
        // BCYA("%s\n", request->request.c_str());
        for (auto arg : request->args) {
            BYEL("arg: %-32s %s\n", arg.first.c_str(), arg.second.c_str());
        }
        BYEL("===============\n");
    // }
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