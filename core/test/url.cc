#include "prizm/prizm.h"
#include "util/url.h"

std::string PUBLIC_DIRECTORY = "./public/frontend";

int main(int argc, char* argv[]) {
    PrizmInit("URL Test");
    
    TEST(URL, Constructor) {
        std::string _url = "https://127.0.0.1:8081/join-model?token=sdk43k5lj3sdf";
        URL* url = new URL(_url);
        _url = "https://127.0.0.1:8081/join-model?token=sdk43k5lj3sdf&get=some&monet=yo&something";
        URL* url2 = new URL(_url);
        _url = "https://dev.jerichoinfrastructure.com/join-model?token=sdk43k5lj3sdf&get=some&monet=yo&something";
        URL* url3 = new URL(_url);
        _url = "https://127.0.0.1:8081/job";
        URL* url4 = new URL(_url);
        _url = "https://127.0.0.1:8081";
        URL* url5 = new URL(_url);
        delete url;
        delete url2;
        delete url3;
        delete url4;
        delete url5;
    }

    PrizmResults();
    PrizmCleanup();
    return 0;   
}