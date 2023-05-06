#ifndef CELERITY_EXTERNAL_URL_H_
#define CELERITY_EXTERNAL_URL_H_

#include <curl/curl.h>

// #define LOG_FAIL Console::Log(FAILURE,
#define OPTS std::vector<std::pair<std::string, std::string>>

class Curl {
  public:

    static size_t writeCallback(char *contents, size_t size, size_t nmemb, void *userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    static const std::string get(const std::string protocol, const std::string url, const std::string route, OPTS options) {
        try {
            CURL *curl;
            CURLcode res;
            // // std::string base = "https://sandbox.iexapis.com/stable/stock/aapl/quote?token=";
            // std::string base = "https://sandbox.iexapis.com/stable/stock/aapl/chart/1m/20211201?token=";
            // std::string base = url;
            std::string base = protocol + "://";
            base += url;
            base += route + "?";
            for (int i = 0; i < options.size(); i++) {
                base += options.at(i).first + "=" + options.at(i).second;
                if (i < options.size() - 1) {
                    base += "&";
                }
            }
            printf("%s\n", base.c_str());
            std::string response;
            curl = curl_easy_init();
            if(curl) {
                curl_easy_setopt(curl, CURLOPT_URL, base.c_str());

                /* example.com is redirected, so we tell libcurl to follow redirection */
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
                // dangerous to add... but iex not responding without it
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

                /* Perform the request, res will get the return code */
                res = curl_easy_perform(curl);
                /* Check for errors */
                if(res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));


                /* always cleanup */
                curl_easy_cleanup(curl);
                return response;
            } else {
                PERR(EINVARG, "Invalid curl call in api!");
            }
        }
        
        catch (std::exception const &e) {
            // LOG_FAIL "Api:Parse -- Failed to parse");
            PERR(EINVARG, "Failed to parse curl");
            std::cerr << e.what() << '\n';
        }

        return "undefined";
    }
};

#endif 