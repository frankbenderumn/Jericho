#define TEST_MODE

#include "prizm/prizm.h"
#include "server/defs.h"
#include "server/request.h"
#include "server/client.h"
#include "util/trace.h"

std::string PUBLIC_DIRECTORY = "./public/frontend";

void atexit_fish() { TRACESCAN; }

int main(int argc, char* argv[]) {
    const int result_1 = std::atexit(atexit_fish);
    PrizmInit("Request");
    std::string r1 = JFS::read("./core/test/data/request_1.txt");
    std::string r2 = JFS::read("./core/test/data/request_2.txt");
    std::string rContent = JFS::read("./core/test/data/request_content.txt");
    std::string r4 = JFS::read("./core/test/data/request_4.txt");
    std::string rPost = JFS::read("./core/test/data/request_post.txt");

    Client* clients = NULL;
    TEST(Request, Eval) {
        Client* cli = create_client(&clients);
        traverse_clients(&clients);
        if (clients == NULL) {
            BRED("Clients still null\n");
        }
        strncpy(cli->request, r1.c_str(), r1.size());
        Request* req = new Request(cli, MAX_REQUEST_SIZE, true);
        req->eval();
        req->validate();
        ASSERT_STR(req->method, "GET");
        ASSERT_STR(req->path, "/assets/images/vr-guy.png");
        REFUTE_STR(req->protocol, "HTTP/1.1\r");
        ASSERT_STR(req->protocol, "HTTP/1.1");
        REFUTE_STR(req->header("sec-ch-ua-platform"), "\"Windows\"\r");
        ASSERT(req->valid, true);
        delete req;
        destroy_client(cli, &clients);
    }

    TEST(Request, FaultyMethod) {
        Client* cli = create_client(&clients);
        traverse_clients(&clients);
        strncpy(cli->request, r2.c_str(), r2.size());
        Request* req = new Request(cli, MAX_REQUEST_SIZE, true);
        req->eval();
        req->validate();
        REFUTE_STR(req->method, "GET");
        REFUTE_STR(req->path, "/assets/images/vr-guy.png");
        REFUTE_STR(req->protocol, "HTTP/1.1\r");
        ASSERT(req->valid, false);
        delete req;
        destroy_client(cli, &clients);
    }

    TEST(Request, FaultyHeaders) {
        Client* cli = create_client(&clients);
        traverse_clients(&clients);
        strncpy(cli->request, r4.c_str(), r4.size());
        Request* req = new Request(cli, MAX_REQUEST_SIZE, true);
        req->eval();
        req->validate();
        ASSERT_STR(req->method, "GET");
        ASSERT_STR(req->path, "/jericho/index.html");
        ASSERT_STR(req->protocol, "HTTP/1.1");
        ASSERT(req->valid, true);
        delete req;
        destroy_client(cli, &clients);
    }

    TEST(Request, Content) {
        Client* cli = create_client(&clients);
        strncpy(cli->request, rContent.c_str(), rContent.size());
        Request* req = new Request(cli, MAX_REQUEST_SIZE, true);
        req->eval();
        std::string prot = req->parseProtocol();
        std::string content = "I got time this is going to be something exciting";
        ASSERT_STR(req->content, content);
        const char* headers = 
        "GET /jericho/index.html HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Connection: keep-alive\r\n"
        "Content-size: 49\r\n"
        "Cache-Control: max-age=0\r\n"
        "sec-ch-ua: \"Not?A_Brand\";v=\"8\", \"Chromium\";v=\"108\", \"Google Chrome\";v=\"108\"\r\n"
        "sec-ch-ua-mobile: ?0\r\n"
        "sec-ch-ua-platform: \"Windows\"\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.0.0 Safari/537.36\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
        "Sec-Fetch-Site: same-origin\r\n"
        "Sec-Fetch-Mode: navigate\r\n"
        "Sec-Fetch-User: ?1\r\n"
        "Sec-Fetch-Dest: document\r\n"
        "Referer: https://localhost:8080/\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Accept-Language: en-US,en;q=0.9";
        // std::string h(headers);
        ASSERT_STR(req->headersStr, headers);
        delete req;
        destroy_client(cli, &clients);
    }

    TEST(Request, Post) {
        Client* cli = create_client(&clients);
        strncpy(cli->request, rPost.c_str(), rPost.size());
        Request* req = new Request(cli, MAX_REQUEST_SIZE, true);
        req->eval();
        std::string prot = req->parseProtocol();
        ASSERT_STR(req->method, "POST");
        ASSERT_STR(req->content, "username=tankinfranklin&password=pass1234");
        ASSERT_STR(req->arg("username"), "tankinfranklin");
        ASSERT_STR(req->arg("password"), "pass1234");
        delete req;
        destroy_client(cli, &clients);
    }

    PrizmResults();
    PrizmCleanup();
    return 0;
}