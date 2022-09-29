#include "server/request.h"

bool is_valid_request(Client* client) {
    CYA("request in parse: %s\n", client->request);
    int g = strncmp("GET /", client->request, 5);
    int p = strncmp("POST /", client->request, 6);
    if (!g || !p) {
        return true;
    }
    return false;
}

int parse_request(Client* client, Request* request) {
    CYA("request in parse: %s\n", client->request);
    int g = strncmp("GET /", client->request, 5);
    int p = strncmp("POST /", client->request, 6);
    if (g && p) {
        return 0;
    } else {
        if (!g) {
            char* path = client->request + 4; // removes "GET "
            char* end_path = strstr(path, " "); // finds first occurence of " "
            int idx = (int)(path - client->request);
            int idx2 = (int)(end_path - client->request);
            printf("IDX1: %i\n", idx);
            printf("IDX2: %i\n", idx2);
            int max_path_length = 100;
            if (idx2 - idx > max_path_length - 1) {
                printf("Path is too long\n");
                return 0;
            } 
            char subbuf[idx2 - idx + 1];
            strncpy(subbuf, path, idx2 - idx);
            subbuf[max_path_length] = '\0';
            char* headers = strstr(end_path, "\r\n");
            if (!end_path) {
                return 0;
            } else {
                request->method = "get";
                request->path = std::string(subbuf);
                request->request = std::string(headers + 2);
                std::vector<std::string> headers = tokenize(request->request, "\r\n");
                for (auto h : headers) {
                    std::vector<std::string> kv = tokenize(h, ": ");
                    request->headers[kv[0]] = kv[1]; 
                }
                *end_path = 0; // zero out char
                return 1; // static file serving
            }
        } else if (!p) {
            char* path = client->request + 5; // removes "POST "
            char* end_path = strstr(path, " "); // finds first occurence of " "
            if (!end_path) {
                return 0;
            } else {
                request->method = "post";
                request->path = std::string(end_path);
                request->request = std::string(client->request);
                std::vector<std::string> headers = tokenize(request->request, "\r\n");
                for (auto h : headers) {
                    std::vector<std::string> kv = tokenize(h, ": ");
                    request->headers[kv[0]] = kv[1]; 
                }
                *end_path = 0; // zero out char
                return 1; // static file serving
            }
        }
    }
    return 0;
}

void print_request(Request* request) {
    BYEL("===============\n");
    BYEL("   REQUEST\n");
    BYEL("---------------\n");
    BYEL("method: %s\n", request->method.c_str());
    BYEL("path: %s\n", request->path.c_str());
    for (auto head : request->headers) {
        BYEL("header: %-32s %s\n", head.first.c_str(), head.second.c_str());
    }
    BYEL("request: \r\n"); 
    BCYA("%s\n", request->request.c_str());
    BYEL("===============\n");
}