#include "prizm2/prizm.h"
#include "util/url.h"

class URLUnit : public ITest {
    MTRACE(URLUnit);
  protected:
    void precondition() {
    }

    void postcondition() {
    }

    std::string bin_url = "bin+https://127.0.0.1:8080/binary-rpc-job";
    std::string json_url = "json+https://127.0.0.1:8080/binary-rpc-job";
    std::string _url1 = "https://127.0.0.1:8081/join-model?token=sdk43k5lj3sdf";
    std::string _url2 = "https://127.0.0.1:8081/join-model?token=sdk43k5lj3sdf&get=some&monet=yo&something";
    std::string _url3 = "https://dev.jerichoinfrastructure.com/join-model?token=sdk43k5lj3sdf&get=some&monet=yo&something";
    std::string _url4 = "https://127.0.0.1:8081/job";
    std::string _url5 = "https://127.0.0.1:8081";

};

PTEST(URLUnit, binary_url) {
    URL* url = new URL(bin_url);
    PASSERT(url->type, "bin");
    PASSERT(url->protocol, "https");
    PASSERT(url->host, "127.0.0.1");
    PASSERT(url->port, "8080");
    PASSERT(url->path, "/binary-rpc-job");
    delete url;
}

PTEST(URLUnit, json_url) {
    URL* url = new URL(json_url);
    PASSERT(url->type, "json");
    PASSERT(url->protocol, "https");
    PASSERT(url->host, "127.0.0.1");
    PASSERT(url->port, "8080");
    PASSERT(url->path, "/binary-rpc-job");
    delete url;
}

PTEST(URLUnit, url1) {
    URL* url = new URL(_url1);
    PASSERT(url->type, "plain");
    PASSERT(url->protocol, "https");
    PASSERT(url->host, "127.0.0.1");
    PASSERT(url->port, "8081");
    PASSERT(url->path, "/join-model");
    std::string val;
    bool has_key_tok = url->arg(val, "tok");
    PASSERT(has_key_tok, false);
    bool has_key_token = url->arg(val, "token");
    PASSERT(has_key_token, true);
    PASSERT(val.length(), 13);
    PASSERT(val.size(), 14);
    std::string res = std::string("sdk43k5lj3sdf");
    bool match = true;
    for (int i = 0; i < res.size(); i++) {
        if (val[i] != res[i]) {
            BRED("Failed to match: [%i] %c with [%i] %c\n", i, val[i], i, res[i]);
            match = false;
        }
    }
    PASSERT(match, true);
    // PASSERT(url->arg(), "/join-model");
    delete url;
}

PTEST(URLUnit, url2) {
    URL* url = new URL(_url2);
    PASSERT(url->type, "plain");
    delete url;
}
