#ifndef SERVER_RESPONSE_H_
#define SERVER_RESPONSE_H_

#include <string.h>

#include "server/defs.h"
#include "server/response_defs.h"
// #include "server/request.h"
#include "session/cookie.h"
#include "message/message_broker.h"
#include "picojson.h"

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

    static std::string ws(int status, std::string response, std::string command, picojson::object* obj = NULL) {
        picojson::object res; 
        res["status"] = picojson::value(std::to_string(status));
        res["message"] = picojson::value(response);
        res["command"] = picojson::value(command);
        if (obj != NULL) {
            res["obj"] = picojson::value(*obj);
        }
        picojson::value v(res);
        return v.serialize();
    }
};

struct Request;
// http response
struct Response {
    Client* client;
    Request* request;
    System* sys;
    bool redirect = false;
    std::vector<Cookie*> cookies;
    std::unordered_map<std::string, std::string> headers;
    Response(System* sys, Request* req);
    ~Response() { PDESTROY; }
    void serve();
    std::string generate();
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