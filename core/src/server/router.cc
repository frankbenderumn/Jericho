#include "server/router.h"

// namespace parser2 {

    // std::string parse(std::string uri) {

    //     if (uri == "/user/$id") {

    //     } else if (uri == "/stock/$id") {

    //     } else if (uri == "/transaction/$id") {

    //     } else if (uri == "/ping") {

    //     } else if (uri == "/pong")

    // }

    /** TODO: switch to large scale response parse (send_response(code, socket)) */ 
    void parser2::parse(Client* client, Client** clients) {
        CYA("request in parse: %s\n", client->request);
        int g = strncmp("GET /", client->request, 5);
        int p = strncmp("POST /", client->request, 6);
        if (g && p) {
            resource::error(client, "404");
        } else {
            if (!g) {
                char* path = client->request + 4; // removes "GET "
                char* end_path = strstr(path, " "); // finds first occurence of " "
                if (!end_path) {
                    resource::error(client, "404");

                    // std::string response = resource::error("404");
                    // serve(client, resource.c_str());
                } else {
                    *end_path = 0; // zero out char
                    resource::serve_cxx(client, clients, path); // static file serving
                }
            } else if (!p) {
                // char* path = client->request + 5; // removes "GET "
                // char* end_path = strstr(path, " "); // finds first occurence of " "
                // if (!end_path) {
                //     send_400(client); // none terminating path
                // } else {
                //     *end_path = 0; // zero out char
                    // serve_resource(client, path); // static file serving
                    post_resource(client, client->request);
                // }
            }
        }
    }

// }