#include "server/resource.h"

void resource::error(Client* client, const std::string& path) {
    std::string s;
    if (path == "404") {
        s = FileSystem::read("resource/error/404.html");
    } else if (path == "500") {
        s = FileSystem::read("resource/error/500.html");            
    } else {
        s = FileSystem::read("resource/error/404.html");
    }
    std::string header = std::string("HTTP/1.1 404 Not Found\r\n"
                    "Connection: close\r\n"
                    "Content-length: ") + std::to_string(s.size()) + "\r\n\r\n";
    std::string toSend = header + s;

    BYEL("TOOOOOO SEND: %s\n", toSend.c_str());

    serve(client, toSend.c_str());
}