#ifndef CELERITY_EXTERNAL_POLYGON_H_
#define CELERITY_EXTERNAL_POLYGON_H_

#include <time.h>
#include <string>
#include <unordered_map>
#include <regex>
#include <deque>
#include <sys/wait.h>

#include <picojson.h>
#include "api/url.h"
#include "api/api.h"
#include "server/response.h"
#include "server/client.h"
#include "system/system.h"
#include "util/iters.h"
#include "prizm/prizm.h"
#include "util/file_system.hpp"
#include "message/callback.h"

typedef std::vector<std::pair<std::string, std::string>> Opts;

class PolygonClient {
  public:
    static std::string send(std::string route, OPTS opts) {
        std::string _protocol = "https";
        std::string _token = "ST5s4i1XvOCXb9wiQvmE9EuBnu34LnJ7";
        std::string _url = "api.polygon.io/";
        Opts _options = {OPT{"apiKey", _token}};
        for (auto opt : opts) {
            _options.push_back(opt);
        }
        std::string result = Curl::get(_protocol, _url, route, _options);
        return result;
    }

    // https://api.polygon.io/v1/open-close/AAPL/2020-10-14?adjusted=true&apiKey=ST5s4i1XvOCXb9wiQvmE9EuBnu34LnJ7
    static std::string ohlc(std::string ticker, std::string date) {
        std::string path = "v1/open-close/" + ticker + "/" + date;
        std::string result = PolygonClient::send(path, 
                                                {OPT{"adjusted", "false"}});
        return result;
    }

    // v2/aggs/grouped/locale/us/market/stocks/2020-10-14?adjusted=true&apiKey=*
    static std::string grouped(std::string date) { 
        std::string path = "v2/aggs/grouped/locale/us/market/stocks/" + date;
        std::string result = PolygonClient::send(path, 
                                                {OPT{"adjusted", "false"}});
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
        std::string path = "v2/aggs/ticker/"+
                            ticker+"/range/"+
                            multiplier+"/"+span+"/"+to+"/"+from;
        std::string result = PolygonClient::send(path, 
                                                {OPT{"adjusted", adjusted}, 
                                                OPT{"sort", sort}, 
                                                OPT{"limit", limit}});
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

std::string apiServeModelJson(Args args) {
    // if (router == NULL) {
    //     return JsonResponse::error(500, "Cluster not in scope of api call");
    // }
    BYEL("Reading Model... torch.pt\n");
    std::string bytes = Jericho::FileSystem::readBinary("./public/cluster/8081/torch.pt");
    BYEL("BYTES: %s\n", bytes.c_str());
    return JsonResponse::success(200, bytes);
}

std::string apiOhlc(Args args) {
    if (args.size() != 2) {
        return "{\"status\": \"500\", \"error\": \"2 arguments required\"}";
    } else {
        return PolygonClient::ohlc(args["ticker"], args["date"]);
    }    
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
}

std::string apiAggregate(Args args) {
    if (args.size() != 8) {
        return "{\"status\": \"500\", \"error\": \"8 arguments required\"}";
    } else {
        return PolygonClient::aggregate(args["ticker"], 
                                        args["multiplier"], 
                                        args["span"], 
                                        args["to"], 
                                        args["from"], 
                                        args["true"], 
                                        args["asc"], 
                                        args["120"]);
    }
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
}

std::string apiMacd(std::vector<std::string> args) {
    if (args.size() != 6) {
        return "{\"status\": \"500\", \"error\": \"8 arguments required\"}";
    } else {
        return PolygonClient::macd(args[0],
                                    args[1], 
                                    args[2], 
                                    args[3], 
                                    args[4], 
                                    args[5], 
                                    args[6], 
                                    args[7]);
    }
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
}

std::string apiSma(Args args) {
    if (args.size() != 6) {
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
    } else {
        return PolygonClient::sma(args["ticker"], 
                                    args["span"], 
                                    args["adjusted"], 
                                    args["window"], 
                                    args["seriesType"], 
                                    args["order"]);
    }
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
}

std::string apiEma(Args args) {
    if (args.size() != 6) {
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
    } else {
        return PolygonClient::ema(args["ticker"],
                                    args["span"], 
                                    args["adjusted"], 
                                    args["window"], 
                                    args["seriesType"], 
                                    args["order"]);
    }
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
}

// https://api.polygon.io/v1/indicators/rsi/AAPL?timespan=day&adjusted=true&window=14&series_type=close&order=desc&apiKey=*
std::string apiRsi(Args args) {
    std::unordered_map<std::string, std::string> v = {
        {"timespan", "day"},
        {"adjusted", "true"},
        {"window", "14"},
        {"series_type", "close"},
        {"order", "desc"},
        {"ticker", "AAPL"},
        {"token", "undefined"}
    }; 
    std::set<std::string> rsiSet = std::set<std::string>{
        "timespan",
        "adjusted",
        "window",
        "series_type",
        "ticker"
    };
    if (subset(rsiSet, keys(args))) {
        if (contains(TOKEN_LIST, args["token"])) {
            for (auto arg : args) {
                v[arg.first] = arg.second;
            }
            std::string result = PolygonClient::rsi(v["ticker"],
                                        v["timespan"],
                                        v["adjusted"],
                                        v["window"],
                                        v["series_type"],
                                        "desc");
            return JsonResponse::success(200, result);
        } else {
            return JsonResponse::error(404, "Invalid token provided");
        }
    } else {
        print(keys(args));
        return JsonResponse::error(500, "invalid arguments provided");
    }
    return JsonResponse::error(500, "Internal Error");
}

#endif