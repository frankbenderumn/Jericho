#ifndef CELERITY_EXTERNAL_POLYGON_H_
#define CELERITY_EXTERNAL_POLYGON_H_

#include <time.h>
#include <string>
#include <unordered_map>
#include <regex>
#include <deque>

#include <picojson.h>
#include "api/url.h"
#include "api/api.h"
#include "server/response.h"
#include "router/router.h"
#include "util/iters.h"

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
        std::string result = Url::get(_protocol, _url, route, _options);
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
        return PolygonClient::aggregate(args[0], 
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

std::string apiSma(std::vector<std::string> args) {
    if (args.size() != 6) {
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
    } else {
        return PolygonClient::sma(args[0], 
                                    args[1], 
                                    args[2], 
                                    args[3], 
                                    args[4], 
                                    args[5]);
    }
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
}

std::string apiEma(std::vector<std::string> args) {
    if (args.size() != 6) {
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
    } else {
        return PolygonClient::ema(args[0],
                                    args[1], 
                                    args[2], 
                                    args[3], 
                                    args[4], 
                                    args[5]);
    }
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
}

std::string apiRsi(std::vector<std::string> args) {
    if (args.size() != 6) {
        return "{\"status\": \"500\", \"error\": \"6 arguments required\"}";
    } else {
        return PolygonClient::rsi(args[0],
                                    args[1],
                                    args[2],
                                    args[3],
                                    args[4],
                                    args[5]);
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
    std::set<std::string> rsiSet = std::set<std::string>{
        "timespan",
        "adjusted",
        "window",
        "series_type",
        "order",
        "ticker",
        "token"
    }
    if (subset(keys(args), rsiSet)) {
        if (contains(TOKEN_LIST, args["token"])) {
            for (auto arg : args) {
                v[arg.first] = arg.second;
            }
            return PolygonClient::rsi(v["ticker"],
                                        v["timespan"],
                                        v["adjusted"],
                                        v["window"],
                                        v["series_type"],
                                        "desc");
        } else {
            return JsonResponse::error(404, "Invalid token provided");
        }
    } else {
        print(keys(args));
        return JsonResponse::error(500, "invalid arguments provided");
    }
}

std::string apiMongoDatabases(Args args,
                            Router* router, 
                            Client* client = NULL) {
    if (!contains(TOKEN_LIST, args["token"])) {
        return JsonResponse::error(404, "Invalid token provided");
    }

    std::vector<std::string> result = router->celerity()->mongo()->databases();

    return JsonResponse::success(200, "Suceesfully return mongo databases");

}

std::string apiMongoInsert(Args args, Router* router, Client* client = NULL) {
    if (!contains(TOKEN_LIST, args["token"])) {
        return JsonResponse::error(404, "Invalid token provided");
    }

    std::vector<std::string> result = router->celerity()->mongo()->databases();

    return JsonResponse::success(200, "Suceesfully return mongo databases");

}

std::string apiMongoCollection(Args args, Router* router, Client* client = NULL) {
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

std::string apiPython(std::unordered_map<std::string, std::string> args, Router* router, Client* client = NULL) {
    if (!contains(TOKEN_LIST, args["token"])) {
        return JsonResponse::error(404, "Invalid token provided");
    }

    if (args.find("command") == args.end()) {
        return JsonResponse::error(404, "Command not provided");
    }

    // #define BUF_MAX 1000000

    FILE *fp;
    int status;
    // char path[BUF_MAX];
    std::string result;

    std::string command = args["command"];

    std::string clients = "";
    if (containsKey(args, std::string("clients"))) {
        clients = args["clients"];
    }

    std::string command_path = "python3 ./py/" + command + ".py " + clients;

    fp = popen(command_path.c_str(), "r");
    if (fp == NULL) { return JsonResponse::error(404, "Invalid command provided"); }

    // printf("Size of pipe: %i\n", (int)sz);
    // BLU("Size of fp: %i\n", sizeof(fp));

    int someLen = 0;
    int lineLen = 10000;
    char *line;
    line = (char *)malloc(sizeof(char) * lineLen);
    if (line == NULL) {
        // Something went horribly wrong
        // exit(1);
    }
    while (fgets(line, lineLen, fp)) {
        // Do something to find the size
        BMAG("LINE: %s\n", line);
        result += std::string(line);
        someLen += lineLen;
        if (someLen != lineLen) {
            line = (char *)realloc(line, sizeof(char) * someLen);
        }
        if (line == NULL) {
            // Something went horribly wrong
            // exit(1);
            BRED("LINE IS NULL\n");
        }
    }

    // fseek(fp, 0L, SEEK_END);
    // size_t sz = ftell(fp);
    // rewind(fp);

    // while (fgets(path, BUF_MAX, fp) != NULL) {
    //     printf("%s", path);
    //     result += std::string(path);
    // }
    printf("Size of line is: %i\n", (int)result.size());

    // result = std::string(line);
    BGRE("LINE: %s\n", result.c_str());

    free(line);

    status = pclose(fp);
    if (status == -1) {
        /* Error reported by pclose() */
        return JsonResponse::error(404, "Invalid command provided");
    }

    BGRE("DONE");

    return JsonResponse::success(200, result);

}

std::string apiSpawn(std::unordered_map<std::string, std::string> args, Router* router = NULL, Client* client = NULL) {
    BBLU("ATTEMPTING TO SPAWN NEW SERVER\n");
    if (!contains(TOKEN_LIST, args["token"])) {
        return JsonResponse::error(404, "Invalid token provided");
    }

    if (!subset(std::set<std::string>{"port", "threads", "directory"}, keys(args))) {
        return JsonResponse::error(404, "Invalid arguments provided");
    }

    if (!apiValidate(args["port"], std::regex("^[0-9][0-9]{0,4}[0-9]$"))) {
        return JsonResponse::error(404, "Port is not a number. Should check if in use.");
    }

    if (!apiValidate(args["threads"], std::regex("^[0-9][0-9]{0,3}[0-9]$|^[0-9]$"))) {
        return JsonResponse::error(404, "Thread is not a number");
    }

    if (std::stoi(args["threads"]) > 10) {
        return JsonResponse::error(404, "Threads must be <= 10");
    }

    if (apiValidate(args["directory"], std::regex("\\.\\.|\\s"))) {
        return JsonResponse::error(404, "Directory contains .. or space");
    }

    if (args["directory"].size() > 100) {
        return JsonResponse::error(500, "Directory length greater than 100");
    }

    std::string directory;
    BYEL("PRE-DIRECTORY: %s\n", args["directory"].c_str());
    if (args["directory"].find("%2f") != std::string::npos){
        std::vector<std::string> chunks = tokenize(args["directory"], "%2F");
        for (auto c : chunks) {
            YEL("CHUNK: %s\n", c.c_str());
        }
        directory = join(chunks, "/");
    }

    BBLU("DIRECTORY TO SPAWN IS: %s\n", directory.c_str());

    std::string command;
    std::string port = args["port"];
    std::string threads = args["threads"];
    if (directory != "") {
        command = "./bin/extend -p " + port + " -t " + threads + " -d \"" + directory + "\"";
    } else {
        command = "./bin/extend -p " + port + " -t " + threads;
    }

    int i = system(command.c_str());

    // if (result != "PIPE ERROR") {
        ClusterNode* node = new ClusterNode("127.0.0.1", port, directory, router->cluster()->index());
        router->cluster()->addNode(node);
        router->cluster()->index()->dump();
    // }

    // only allow 5 threads for now until proper error checking and conversion

    return JsonResponse::success(200, "Spawned a new server at host:" + port);
}

std::string apiPingOne(Args args, Router* router = NULL, Client* client = NULL) {
    if (!contains(TOKEN_LIST, args["token"])) {
        return JsonResponse::error(404, "Invalid token provided");
    }

    if (router == NULL || client == NULL) {
        return JsonResponse::error(500, "Cluster not in scope of api call");
    }

    if (!containsKey(args, std::string("port"))) {
        return JsonResponse::error(404, "Port needs to be specified");
    }

    ClusterNode* node = router->cluster()->boss()->getEdge("127.0.0.1", args["port"]);


    if (node != nullptr) {
        BYEL("Selected Node, Host: %s, Port: %s\n", node->host().c_str(), node->port().c_str());
        router->cluster()->pingOne(router, client, node);
    } else {
        return JsonResponse::error(404, "Host and port are not part of cluster. Spawn instance first");
    }

    return "TICKET";
}

std::string apiPingAll(Args args, Router* router = NULL, Client* client = NULL) {
    if (!contains(TOKEN_LIST, args["token"])) {
        return JsonResponse::error(404, "Invalid token provided");
    }

    if (router == NULL || client == NULL) {
        return JsonResponse::error(500, "Cluster not in scope of api call");
    }


    router->cluster()->pingAll(router, client);

    return "TICKET";
}

std::string apiPingLocal(Args, Router* router = NULL, Client* client = NULL) {
    std::string host = router->cluster()->boss()->host();
    std::string port = router->cluster()->boss()->port();
    return "Hello from " + host + ":" + port;
}

std::string concatBuf(Router* router, Client* client, std::deque<MessageBuffer*> mq) {
    std::string result;
    for (auto m : mq) {
        result += m->received + ":";
    }
    BCYA("Result: %s\n", result.c_str());
    return result;
}

std::string apiFederateLocal(Args args, Router* router = NULL, Client* client = NULL) {
    if (router == NULL) {
        return JsonResponse::error(500, "Cluster not in scope of api call");
    }

    std::string host = router->cluster()->boss()->host();
    std::string port = router->cluster()->boss()->port();

    if (containsKey(args, std::string("content"))) {
        return "Federating from " + host + ":" + port + ". Received message " + args["content"];
    } else {
        return "Federating from " + host + ":" + port + ". No message sent.";
    }
}

#endif