#ifndef CELERITY_EXTERNAL_IEX_H_
#define CELERITY_EXTERNAL_IEX_H_

#include <picojson.h>
#include "api/api.h"
#include <time.h>
// #include "util/parser/json_parser.h"

class Iex {
  public:
    Iex() {}
    Iex(std::string token) { _token = token; }

    ~Iex() { printf("deleting iex client\n"); }

    void exporter(const std::string& ticker) {
        std::string route = "stock/" + ticker + "/quote";
        std::string s = getenv("IEX_TOKEN");
        BYEL("IEX TOKEN TEST: %s\n", s.c_str());
        std::string json = Url::Get(_base, route, getenv("IEX_TOKEN"));
        std::string timestamp = this->timestamp();
        std::string dumpPath = "data/finance/" + ticker + "-" + timestamp + ".json";
        std::ofstream stream(dumpPath.c_str());
        stream << json << std::endl;
    }

    picojson::value quote(const std::string ticker) {
        // if (validate()) {
        std::string route = "stock/" + ticker + "/quote";
        std::string json = Url::Get(_base, route, getenv("IEX_TOKEN"));
        picojson::value data;
        std::string err = picojson::parse(data, json);
        if (!err.empty()) {
            std::cerr << err << std::endl;
        } else {
            printf("json data parsed\n");
        }
        return data;
        // }
    }

    std::string historical(const std::string ticker, const std::string range, const std::string date) {
        // if (validate()) {
            std::string route = "stock/" + ticker + "/chart/" + range + "/" + date;
            std::string result = Url::Get(_base, route, getenv("IEX_TOKEN"));
            picojson::value data;
            std::string err = picojson::parse(data, result);
            if (!err.empty()) {
                std::cerr << err << std::endl;
            } else {
                printf("json data parsed\n");
            }
            // result = data.serialize();
            std::string timestamp = this->timestamp();
            std::string dumpPath = "data/finance/" + ticker + "-" + timestamp + ".json";
            std::ofstream stream(dumpPath.c_str());
            std::cout << data.serialize() << std::endl;
            stream << data << std::endl;
            return result;
        // }
    }

    std::string volume(const std::string ticker, const std::string range, const std::string date) {
        // if (validate()) {
            std::string route = "stock/" + ticker + "/chart/" + range + "/" + date;
            std::string result = Url::Get(_base, route, getenv("IEX_TOKEN"));
            std::string timestamp = this->timestamp();
            std::string dumpPath = "data/finance/" + ticker + "-" + timestamp + ".json";
            std::ofstream stream(dumpPath.c_str());
            std::cout << result << std::endl;
            stream << result << std::endl;
            return result;
        // }
    }


  private:
    bool validate() {
        // return (_token != "undefined");
        return true;
    }

    std::string timestamp() {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        char time[128];
        snprintf(time, 128, "%d_%02d_%02d_%02d_%02d_%02d", 
                            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                            tm.tm_hour, tm.tm_min, tm.tm_sec);
        std::string result(time);
        return result;
    }

    // use cloud for production
    std::string _base = "https://sandbox.iexapis.com/stable/";
    std::string _token = "undefined";
};

    // std::string base = "https://sandbox.iexapis.com/stable/stock/aapl/quote?token=";
    // std::string base = "https://sandbox.iexapis.com/stable/stock/aapl/chart/1m/20211201?token=";


        // std::string token = "Tpk_f83b0edb1ba24de8b19d58d61ddf13dc";
        // Iex* client = new Iex(token);
        // picojson::value iex = client->Quote("AAPL");

        // if (iex.is<picojson::array>()) {
        //     Console::Log(SUCCESS, "iex data is an array");
        //     picojson::array arr = iex.get<picojson::array>();
        //     std::cout << "array size of: " << arr.size() << std::endl;
        //     int iteration = 0;
        //     for (int i = 0; i < arr.size(); i++) {
        //         int prefix = 0;
        //         // std::cout << "Iteration -- " << iteration << std::endl;
        //         if (arr[i].is<picojson::object>()) {
        //             picojson::object o = arr[i].get<picojson::object>();
        //             for (picojson::object::const_iterator it = o.begin(); it != o.end(); it++) {
        //                 // std::cout << prefix << " --- " << it->first << ": " << it->second << std::endl;
        //                 prefix++;
        //             }
        //             // std::cout << std::endl;
        //             iteration++;

        //         } else {
        //             std::cout << "is not an object" << std::endl;
        //         }
        //     }
        // } else if (iex.is<picojson::object>()) {
        //     Console::Log(SUCCESS, "iex data is an object");
        //     picojson::object o = iex.get<picojson::object>();
        //     int prefix = 0;
        //     for (picojson::object::const_iterator it = o.begin(); it != o.end(); it++) {
        //         // std::cout << prefix << " --- " << it->first << ": " << it->second << std::endl;
        //         prefix++;
        //     }
        // } else {
        //     Console::Log(FAILURE, "Invalid picojson data from iex");
        // }

        // delete client;

#endif
