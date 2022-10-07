#ifndef CELERITY_EXTERNAL_POLYGON_H_
#define CELERITY_EXTERNAL_POLYGON_H_

#include <picojson.h>
#include "api/url.h"
#include <time.h>
#include <string>

#define OPT std::pair<std::string, std::string>

const std::unordered_map<std::string, int> TOKEN_LIST = {
    {"kalmoru", 0},
    {"assinine", 0},
    {"ass", 0},
    {"titties", 0},
    {"kobutcha", 0}
};

template <typename K, typename V>
bool contains(std::unordered_map<K, V> map, K key) {
    if (map.find(key) != map.end()) return true;
    return false;
}

class PolygonClient {
  public:
    static std::string send(std::string route, OPTS opts) {
        std::string _protocol = "https";
        std::string _token = "ST5s4i1XvOCXb9wiQvmE9EuBnu34LnJ7";
        std::string _url = "api.polygon.io/";
        std::vector<std::pair<std::string, std::string>> _options = {OPT{"apiKey", _token}};
        for (auto opt : opts) {
            _options.push_back(opt);
        }
        std::string result = Url::get(_protocol, _url, route, _options);
        return result;
    }

    // https://api.polygon.io/v1/open-close/AAPL/2020-10-14?adjusted=true&apiKey=ST5s4i1XvOCXb9wiQvmE9EuBnu34LnJ7
    static std::string ohlc(std::string ticker, std::string date) {
        std::string result = PolygonClient::send("v1/open-close/" + ticker + "/" + date, {OPT{"adjusted", "false"}});
        return result;
    }

    // v2/aggs/grouped/locale/us/market/stocks/2020-10-14?adjusted=true&apiKey=*
    static std::string grouped(std::string date) { 
        std::string result = PolygonClient::send("v2/aggs/grouped/locale/us/market/stocks/" + date, {OPT{"adjusted", "false"}});
        return result;
    }

    // v2/aggs/ticker/AAPL/range/1/day/2021-07-22/2021-07-22?adjusted=true&sort=asc&limit=120&apiKey=*
    static std::string aggregate(std::string ticker, 
                            std::string multiplier, 
                            std::string span, 
                            std::string to, 
                            std::string from, 
                            std::string adjusted = "true", 
                            std::string sort = "asc", 
                            std::string limit = "120") {
        std::string result = PolygonClient::send("v2/aggs/ticker/"+ticker+"/range/"+multiplier+"/"+span+"/"+to+"/"+from, 
                                {OPT{"adjusted", adjusted}, OPT{"sort", sort}, OPT{"limit", limit}});
        return result;
    }

    // https://api.polygon.io/v1/indicators/ema/AAPL?timespan=day&adjusted=true&window=50&series_type=close&order=desc&apiKey=*
    static std::string ema(std::string ticker,
                std::string span = "day", 
                std::string adjusted = "true", 
                std::string window = "50", 
                std::string seriesType = "close", 
                std::string order = "desc") { 
    std::string result = PolygonClient::send("v1/indicators/ema/"+ticker, 
                        {OPT{"timespan", span}, 
                        OPT{"adjusted", adjusted},
                        OPT{"window", window},  
                        OPT{"series_type", seriesType}, 
                        OPT{"order", order}}
                        );
        return result;
    }

    // https://api.polygon.io/v1/indicators/sma/AAPL?timespan=day&adjusted=true&window=50&series_type=close&order=desc&apiKey=*
    static std::string sma(std::string ticker, 
                    std::string span = "day", 
                    std::string adjusted = "true", 
                    std::string window = "50", 
                    std::string seriesType = "close", 
                    std::string order = "desc") { 
        std::string result = PolygonClient::send("v1/indicators/sma/"+ticker, 
                            {OPT{"timespan", span}, 
                            OPT{"adjusted", adjusted},
                            OPT{"window", window},  
                            OPT{"series_type", seriesType}, 
                            OPT{"order", order}}
                            );
        return result;
    }

    // https://api.polygon.io/v1/indicators/macd/AAPL?timespan=day&adjusted=true&short_window=12&long_window=26&signal_window=9&series_type=close&order=desc&apiKey=*
    static std::string macd(std::string ticker, 
                std::string span = "day", 
                std::string adjusted = "true", 
                std::string shortWindow = "12", 
                std::string longWindow = "26", 
                std::string signalWindow = "9", 
                std::string seriesType = "close", 
                std::string order = "desc") { 
    std::string result = PolygonClient::send("v1/indicators/macd/"+ticker, 
                        {OPT{"timespan", span}, 
                        OPT{"adjusted", adjusted},
                        OPT{"short_window", shortWindow},  
                        OPT{"long_window", longWindow},  
                        OPT{"signal_window", signalWindow},  
                        OPT{"series_type", seriesType}, 
                        OPT{"order", order}}
                        );
        return result;
    }

    // https://api.polygon.io/v1/indicators/rsi/AAPL?timespan=day&adjusted=true&window=14&series_type=close&order=desc&apiKey=*
    static std::string rsi(std::string ticker,
                    std::string span = "day", 
                    std::string adjusted = "true", 
                    std::string window = "14", 
                    std::string seriesType = "close", 
                    std::string order = "desc") { 
        std::string result = PolygonClient::send("v1/indicators/rsi/"+ticker, 
                            {OPT{"timespan", span}, 
                            OPT{"adjusted", adjusted},
                            OPT{"window", window},  
                            OPT{"series_type", seriesType}, 
                            OPT{"order", order}}
                            );
        return result;
    }
};

std::string apiOhlc(std::vector<std::string> args) {
    if (args.size() != 2) {
        return "{\"status\": \"500\", \"error\": \"2 arguments required\"}";
    } else {
        return PolygonClient::ohlc(args[0], args[1]);
    }    
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
}

std::string apiAggregate(std::vector<std::string> args) {
    if (args.size() != 8) {
        return "{\"status\": \"500\", \"error\": \"8 arguments required\"}";
    } else {
        return PolygonClient::aggregate(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
    }
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
}

std::string apiMacd(std::vector<std::string> args) {
    if (args.size() != 6) {
        return "{\"status\": \"500\", \"error\": \"8 arguments required\"}";
    } else {
        return PolygonClient::macd(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
    }
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
}

std::string apiSma(std::vector<std::string> args) {
    if (args.size() != 6) {
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
    } else {
        return PolygonClient::sma(args[0], args[1], args[2], args[3], args[4], args[5]);
    }
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
}

std::string apiEma(std::vector<std::string> args) {
    if (args.size() != 6) {
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
    } else {
        return PolygonClient::ema(args[0], args[1], args[2], args[3], args[4], args[5]);
    }
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
}

std::string apiRsi(std::vector<std::string> args) {
    if (args.size() != 6) {
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
    } else {
        return PolygonClient::rsi(args[0], args[1], args[2], args[3], args[4], args[5]);
    }
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
}

// https://api.polygon.io/v1/indicators/rsi/AAPL?timespan=day&adjusted=true&window=14&series_type=close&order=desc&apiKey=*
std::string apiRsi(std::unordered_map<std::string, std::string> args) {
    std::unordered_map<std::string, std::string> v = {
        {"timespan", "day"},
        {"adjusted", "true"},
        {"window", "14"},
        {"series_type", "close"},
        {"order", "desc"},
        {"ticker", "AAPL"},
        {"token", "undefined"}
    }; 
    if (subset(keys(args), std::set<std::string>{"timespan", "adjusted", "window", "series_type", "order", "ticker", "token"})) {
        if (contains(TOKEN_LIST, args["token"])) {
            for (auto arg : args) {
                v[arg.first] = arg.second;
            }
            return PolygonClient::rsi(v["ticker"], v["timespan"], v["adjusted"], v["window"], v["series_type"], "desc");
        } else {
            return JsonResponse::error(404, "Invalid token provided");
        }
    } else {
        print(keys(args));
        return JsonResponse::error(500, "invalid arguments provided");
    }
}

std::string apiMongoDatabases(std::unordered_map<std::string, std::string> args, Router* router) {
    if (!contains(TOKEN_LIST, args["token"])) {
        return JsonResponse::error(404, "Invalid token provided");
    }

    std::vector<std::string> result = router->celerity()->mongo()->databases();

    return JsonResponse::success(200, "Suceesfully return mongo databases");

}

std::string apiMongoInsert(std::unordered_map<std::string, std::string> args, Router* router) {
    if (!contains(TOKEN_LIST, args["token"])) {
        return JsonResponse::error(404, "Invalid token provided");
    }

    std::vector<std::string> result = router->celerity()->mongo()->databases();

    return JsonResponse::success(200, "Suceesfully return mongo databases");

}

std::string apiMongoCollection(std::unordered_map<std::string, std::string> args, Router* router) {
    std::unordered_map<std::string, std::string> v = {
        {"timespan", "day"},
        {"adjusted", "true"},
        {"window", "14"},
        {"series_type", "close"},
        {"order", "desc"},
        {"ticker", "AAPL"},
        {"token", "undefined"}
    }; 
    if (subset(keys(args), std::set<std::string>{"timespan", "adjusted", "window", "series_type", "order", "ticker", "token"})) {
        if (contains(TOKEN_LIST, args["token"])) {
            for (auto arg : args) {
                v[arg.first] = arg.second;
            }
            return PolygonClient::rsi(v["ticker"], v["timespan"], v["adjusted"], v["window"], v["series_type"], "desc");
        } else {
            return JsonResponse::error(404, "Invalid token provided");
        }
    } else {
        print(keys(args));
        return JsonResponse::error(500, "invalid arguments provided");
    }
}

#endif