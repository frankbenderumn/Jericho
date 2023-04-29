#include "prizm2/prizm.h"
#include "server/request.h"

#undef BOOL
#define BOOL bool

class RequestUnit : public ITest {
    MTRACE(RequestUnit);
  protected:
    void precondition() {
        client = (Client*)malloc(sizeof(Client));
        std::string regular = JFS::read("./core/test/data/request_1.txt");
        strncpy(client->request, regular.data(), regular.size());
        req = new Request(client, 4096, true);
    }

    void postcondition() {
        delete req;
        free(client);
    }

    Client* client = nullptr;
    Request* req = nullptr;
    int a = 0;
    int b = 2;
};

PTEST(RequestUnit, Something) {
    PASSERT(a, 0);
    PBENCH(Okay);
    PSTOP(Okay);
}

PTEST(RequestUnit, Eval) {
    req->eval();
}

PTEST(RequestUnit, Base64Binary) {
    Client* client2 = (Client*)malloc(sizeof(Client));
    std::string regular = JFS::read("./core/test/data/request_bin.txt");
    strncpy(client2->request, regular.data(), regular.size());
    PASSERT(regular.size(), 702);
    std::string binary_string;
    const char* data = "eseweflkwjelkjlk\0ssjkl\0sldfjlsdkjf\0\0\0\0\0\0\0\0\0\0\0\0\0I AM BINARY BAD BITCH";
    size_t length = 68;
    PASSERT(length, 68);
    binary_string.append(data, length);
    std::string encoded = jcrypt::base64::encode_url(binary_string);
    BBLU("Binary string size: %li\n", binary_string.size());
    memcpy(client2->request + regular.size(), encoded.data(), encoded.size());
    PBENCH(BinRequestB64)
    Request* req2 = new Request(client2, 4096, true);
    char* p = strstr(client2->request, "\r\n\r\n");
    std::string headers = std::string(client2->request, p - client2->request);
    std::string binContent = std::string(p+4, 68);
    for (auto& c : binContent) {
        if (c != '\0') {
            printf("%c", c);
        } else {
            printf("\\0");
        }
    }
    printf("\n");
    BYEL("Headers: %s\n", headers.c_str());
    req2->eval();
    req2->dump();
    bool bin_match = true;
    PASSERT(binary_string.size(), req2->content.size());
    // reuse this for strings

    if (binary_string.size() == req2->content.size()) {
        for (int i = 0; i < binary_string.size(); i++) {
            char c = binary_string[i];
            char d = req2->content[i];
            if (c != d) {
                BRED("Character mismatch on idx: %i\n", i);
                bin_match = false;
            }
        }
    } else {
        bin_match = false;
    }
    PASSERT(bin_match, true);
    PSTOP(BinRequestB64)
    free(client2);
    delete req2;
}