#include "session/session.h"
#include "server/client.h"

std::string PUBLIC_DIRECTORY = "./public/frontend";

namespace test {
    bool authenticate(Request* req) {
        std::string user = req->arg("username");
        std::string pass = req->arg("password");
        if (user == "tankinfranklin" && pass == "pass1234") return true;
        return false;
    }
}

void atexit_fish() { TRACESCAN; }

using namespace jericho;

int main(int argc, char* argv[]) {
    const int result_1 = std::atexit(atexit_fish);
    PrizmInit("Session");
    Client* clients = 0;
    SessionManager* sm = new SessionManager;
    std::string r1 = JFS::read("./core/test/data/request_post.txt"); // need error checking on file existence
    std::vector<std::string> secure_paths = {
        "/jericho/denathrius.html",
        "/jericho/login.html"
    };
    Client* cli = create_client(&clients);
    strncpy(cli->request, r1.c_str(), r1.size());
    Request* req = new Request(cli, MAX_REQUEST_SIZE, true);
    req->eval();
    if (prizm::contains(secure_paths, req->path)) {
        if (!sm->sessionValid(req)) {
            if (test::authenticate(req)) {
                BGRE("Login successful. Generating session...\n");
                sm->create(req);
            } else {
                BRED("Invalid login.\n");
            }
        } else {
            BMAG("Currently in valid session!\n");
        }
    }
    TEST(Session, Timeout) {
        ASSERT(sm->sessionValid(req), true);
        ASSERT(sm->sessionCount(), (size_t)1);
        sleep(10);
        ASSERT(sm->sessionValid(req), false);
        ASSERT(sm->sessionCount(), (size_t)0);
    }
    delete sm;
    delete req;
    destroy_client(cli, &clients);
    PrizmResults();
    PrizmCleanup();
    return 0;
}