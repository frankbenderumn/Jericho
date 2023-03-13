#include "server/resource.h"
#include "iris/iris.h"

using namespace Jericho;

// only needs to be Router not system
void resource::error(Router* router, Client* client, const std::string& path) {
    std::string s;
    if (path == "404") {
        s = JFS::read("resource/error/404.html");
    } else if (path == "305") {
        s = JFS::read("resource/error/report.html");
    } else if (path == "500") {
        s = JFS::read("resource/error/500.html");            
    } else {
        s = JFS::read("resource/error/404.html");
    }
    iris::interpret(router, s);
    std::string header = std::string("HTTP/1.1 404 Not Found\r\n"
                    "Connection: close\r\n"
                    "Content-length: ") + std::to_string(s.size()) + "\r\n\r\n";
    std::string toSend = header + s;

    // BYEL("TOOOOOO SEND: %s\n", toSend.c_str());

    serve(client, toSend.c_str());
}