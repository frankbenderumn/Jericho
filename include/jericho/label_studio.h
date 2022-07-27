#ifndef LABEL_STUDIO_API_H_
#define LABEL_STUDIO_API_H_

#include <picojson.h>
#include <iostream>
#include <curl/curl.h>

static const std::string curl_get(const std::string url, const std::string route, const std::string token) {
    try {
        CURL *curl;
        CURLcode res;
        // // std::string base = "https://sandbox.iexapis.com/stable/stock/aapl/quote?token=";
        // std::string base = "https://sandbox.iexapis.com/stable/stock/aapl/chart/1m/20211201?token=";
        // std::string base = url;
        std::string base = url;
        base += route;
        base += "?token=";
        base += token;
        Console::Log(FAILURE, "route is");
        printf("%s\n", base.c_str());
        // Console::Log(INFO, "Base");
        // std::cout << base << std::endl;
        std::string response;
        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, base.c_str());

            /* example.com is redirected, so we tell libcurl to follow redirection */
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
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
            Console::Log(SUCCESS, "Curl retrieved!");
            // std::cout << response << std::endl;
            return response;
        } else {
            Console::Log(FAILURE, "Invalid Curl call");
        }
    }
    
    catch (std::exception const &e) {
        LOG_FAIL "Api:Parse -- Failed to parse");
        std::cerr << e.what() << '\n';
    }
}

std::string LSApi_parse() {
    std::ifstream nodes(path);
    std::stringstream buf;
    buf << nodes.rdbuf();
    std::string json = buf.str();
    return Parse(json);

    picojson::value data;
    std::string err = picojson::parse(data, path);
    if (!err.empty()) {
        std::cerr << err << std::endl;
    } else {
        printf("json data parsed\n");
    }

    if (ls.is<picojson::array>()) {
        // Console::Log(SUCCESS, "iex data is an array");
        picojson::array arr = iex.get<picojson::array>();
        std::cout << "array size of: " << arr.size() << std::endl;
        int iteration = 0;
        for (int i = 0; i < arr.size(); i++) {
            int prefix = 0;
            // std::cout << "Iteration -- " << iteration << std::endl;
            if (arr[i].is<picojson::object>()) {
                picojson::object o = arr[i].get<picojson::object>();
                for (picojson::object::const_iterator it = o.begin(); it != o.end(); it++) {
                    // std::cout << prefix << " --- " << it->first << ": " << it->second << std::endl;
                    prefix++;
                }
                // std::cout << std::endl;
                iteration++;

            } else {
                std::cout << "is not an object" << std::endl;
            }
        }
    } else if (iex.is<picojson::object>()) {
        Console::Log(SUCCESS, "iex data is an object");
        picojson::object o = iex.get<picojson::object>();
        int prefix = 0;
        for (picojson::object::const_iterator it = o.begin(); it != o.end(); it++) {
            // std::cout << prefix << " --- " << it->first << ": " << it->second << std::endl;
            prefix++;
        }
    } else {
        // Console::Log(FAILURE, "Invalid picojson data from iex");
    }
}

#endif