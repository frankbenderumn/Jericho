#include "prizm/prizm.h"
#include "server/request_v2.h"

/* TODO: create asm version and benchmark with this one */

class Url {
    std::string _original;
    std::string _protocol;
  public:
    Url(std::string input) {
        _original = input;
        char buffer[input.size()];
        strncpy(buffer, input.c_str(), input.size());
        buffer[input.size()] = 0;
        MAG("%s\n", buffer);
        if (_parse(buffer) < 0) {
            BRED("Url::constructor failed\n");
        }
    }

    int _parse(const char* buffer) {
        char* ptr = 0;
        char* p = strstr((char*)buffer, "://");
        if (p) {
            WHI("%s\n", p);
            WHI("%s\n", buffer);
            char protocol[p - buffer];
            strncpy(protocol, buffer, p - buffer);
            protocol[p - buffer] = 0;
            WHI("%s\n", protocol);
            _protocol = std::string(protocol);
        } else {
            RED("Invalid Url\n");
            return -1;
        }

        p += 3;
        ptr = strstr(p, ":");
        if (ptr) {
            char hostname[ptr - p];
            strncpy(hostname, p, ptr - p);
            hostname[ptr - p] = 0;
            WHI("hostname: %s\n", hostname);
        } else {
            RED("Invalid Hostname\n");
        }

        p = ptr + 1;
        ptr = strstr(p, "/");
        if (ptr) {
            char port[ptr - p];
            strncpy(port, p, ptr - p);
            port[ptr - p] = 0;
            WHI("port: %s\n", port);
    
        } else {
            RED("Invalid port\n");
        }

        p = ptr + 1;
        ptr = strstr(p, "?");
        if (ptr) {
            char path[ptr - p + 1];
            strcpy(path, "/");
            strncpy(path + 1, p, ptr - p);
            path[ptr - p + 1] = 0;
            WHI("path: %s\n", path);
            YEL("Size of char buffer: %li\n", strlen(buffer));
        } else {
            YEL("Size of char buffer: %li\n", strlen(buffer));
        }
        return 0;
    }

    const std::string stringify() const { return _original; }

    const std::string protocol() const { return _protocol; }
};

class Message {

};

class HttpMessage {

};

class JsonMessage {

};

int main(int argc, char* argv[]) {
    PrizmInit("Url Test");

    TEST(Url, Constructor) {
        Url url("https://localhost:8080/index.html?asp=something-cool&token=hell");
        // ASSERT_STR(url.protocol(), "https");
    }
    
    TEST(Url, Stringify) {
        Url url("Well this sucks");
        ASSERT_STR(url.stringify(), "Well this sucks");
    }
    
    PrizmDump();
    PrizmCleanup();
    return 0;
}
