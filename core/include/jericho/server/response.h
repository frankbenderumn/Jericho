#ifndef SERVER_RESPONSE_H_
#define SERVER_RESPONSE_H_

#include <string.h>

#include "server/defs.h"
#include "server/response_defs.h"

struct JsonResponse {
    static std::string error(int code, std::string message) {
        return "{\"status\": \""+std::to_string(code)+"\", \"error\": \""+message+"\"}";
    }
    static std::string success(int code, std::string message) {
        return "{\"status\": \""+std::to_string(code)+"\", \"success\": \""+message+"\"}";
    }
    static std::string message(std::string message) {
        return "{\"message\": \""+message+"\"}";
    }
};

// intuitive
void send_400(Client* client);

void send_404(Client* client);

void serve_resource(Client* conn, const char* path);

void serve(Client* client, const char* content);

std::string header(Client* client, HttpResponseType hresponse);

/** TODO: switch to large scale response parse (send_response(code, socket)) */ 
void parse_response(Client* client);

void post_resource(Client* conn, char* resource);

#endif