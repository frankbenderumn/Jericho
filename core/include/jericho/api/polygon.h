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
#include "router/router.h"
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
    // if (!contains(TOKEN_LIST, args["token"])) {
    //     return JsonResponse::error(404, "Invalid token provided");
    // }

    if (router == NULL || client == NULL) {
        return JsonResponse::error(500, "Cluster not in scope of api call");
    }

    // for naive approach testing (no predefined topology)
    std::vector<std::pair<std::string, std::string>> set = {
        {"127.0.0.1", "8081"},
        {"127.0.0.1", "8082"},
        {"127.0.0.1", "8083"},
        {"127.0.0.1", "8084"},
        {"127.0.0.1", "8085"},
        {"127.0.0.1", "8086"},
        {"127.0.0.1", "8087"},
        {"127.0.0.1", "8088"},
        {"127.0.0.1", "8089"},
        {"127.0.0.1", "8090"}
    };

    BCYA("PINGING ALL...\n");
    // for predefined topology
    // router->cluster()->pingAll(router, client);

    // for naive topology testing (clients come and go)
    router->cluster()->pingSet(router, client, set);

    return "TICKET";
}

std::string apiPingLocal(Args, Router* router = NULL, Client* client = NULL) {
    std::string host = router->cluster()->boss()->host();
    std::string port = router->cluster()->boss()->port();
    std::string msg = "Hello from " + host + ":" + port;
    return "{\"live\": true, \"response\": \""+msg+"\"}";
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

std::string apiPostgres(Args args, Router* router = NULL, Client* client = NULL) {
    if (subset(keys(args), std::set<std::string>{"table"})) {
        BRED("KEYS ARE NOT SUBSET FOR API POSTGRES\n");
    }
    return JsonResponse::success(200, "Let's go");
}

std::string apiServeModelJson(Args args) {
    // if (router == NULL) {
    //     return JsonResponse::error(500, "Cluster not in scope of api call");
    // }
    BYEL("Reading Model... torch.pt\n");
    std::string bytes = Jericho::FileSystem::readBinary("./public/cluster/8081/torch.pt");
    BYEL("BYTES: %s\n", bytes.c_str());
    return JsonResponse::success(200, bytes);
}

std::string apiServeModel(Args args, Router* router = NULL, Client* client = NULL) {
    if (router == NULL) {
        return JsonResponse::error(500, "Cluster not in scope of api call");
    }

    std::string bytes = Jericho::FileSystem::readBinary("./py/scripts/torch.pt");
    return bytes;
}

#define SYS_CALL(x) std::string x(Args args, Router* router = NULL, Client* client = NULL)
#define API_ARG(args, name) (containsKey(args, name)) ? args[name] : "undefined";

SYS_CALL(apiRequestJoin) {
    BMAG("REQUESTION JOIN...\n");
    if (client == NULL) {
        BRED("CLIENT IS NULL SOMEHOW :: apiJoin\n");
    }
    std::string resource = API_ARG(args, std::string("resource"));
    MessageBuffer* buf = router->cluster()->buffer(client, "/join");
    if (resource != "undefined") {
        buf->flag = resource;    
    }
    buf->port = "8080";
    router->cluster()->boss()->brokerSend(router, client, "/join", buf);
    return "TICKET";
    // }
    // return JsonResponse::error(500, "Failed to join federation");
}

SYS_CALL(apiJoin) {
    BMAG("JOINING...\n");
    if (client == NULL) {
        BRED("CLIENT IS NULL SOMEHOW :: apiJoin\n");
    }
    // char address_buffer[16];
    // uint16_t p;
	// client_get_full_address(client, address_buffer, &p);
    // std::string port = std::to_string(p);
    // std::string host(address_buffer);
    std::string host;
    std::string port;
    std::string h = API_ARG(args, std::string("Host"));
    if (h != "undefined") {
        std::vector<std::string> vec = tokenize(h, ':');
        host = vec[0];
        port = vec[1];
    }
    // std::string port = API_ARG(args, port);
    // BBLU("Client address is %s, port is %s\n", host.c_str(), port.c_str());
    if (router->needsAggregate()) {
        ClusterQuorum* q = router->cluster()->index()->get(host, port);
        if (q == nullptr) {
            if (router->cluster()->join(host, port)) {
                BGRE("JOINING NEW QUORUM\n");
                for (auto a : router->cluster()->index()->quorum()) {
                    if (a.second == nullptr) {
                        BRED("NULL QUORUM FOUND\n");
                        continue;
                    }
                    BBLU("%s :: %s", a.first.c_str(), a.second->serialize().c_str());
                }
                q = router->cluster()->index()->get(host, port);
            } else {
                BRED("FAILED TO JOIN -- SHOULD NEVER HAPPEN!\n");
                return "{\"status\": 500, \"response\": {\""+port+"\": {\"status\": \"failed\", \"message\": \"Failed to join federation - CRITICAL\"}}}";
            }
        }
        if (q == nullptr) {
            BRED("QUORUM STILL NULL\n");
        }
        q->status = FL_JOINED;
        router->cluster()->index()->quorumTrain(std::vector<ClusterQuorum*>{q});
        BYEL("SIZE OF QUORUM IS: %li\n", router->cluster()->index()->quorum().size());
        for (auto a : router->cluster()->index()->quorum()) {
            if (a.second == nullptr) {
                BRED("NULL QUORUM FOUND\n");
                continue;
            }
            BBLU("%s", a.second->port.c_str());
        }
        router->cluster()->boss()->send2(router, client, "127.0.0.1:"+port+"/get-fed-model");
        return "{\"status\": 200, \"response\": {\""+port+"\": {\"status\": \"joined\", \"message\": \"Requesting model\"}}}";
    } else {
        // if (router->cluster()->join(host, port)) {
        //     return "{\"status\": 200, \"response\": {\""+port+"\": {\"status\": \"joined\", \"message\": \"Successfully joined federation\"}}}";
        // }
        // return "{\"status\": 500, \"response\": {\""+port+"\": {\"status\": \"failed\", \"message\": \"Failed to join federation\"}}}";
    }
    return "{\"status\": 500, \"response\": {\""+port+"\": {\"status\": \"failed\", \"message\": \"Failed to join federation\"}}}";
}

API(FederationStatus, {}) 
    BYEL("GETTING FEDERATION STATUS...\n");
}

API(GetFedModel, {})
    BMAG("GETTING FED MODEL...\n");
    std::string h = API_ARG(args, std::string("Host"));
    std::string host;
    std::string port;
    if (h != "undefined") {
        std::vector<std::string> vec = tokenize(h, ':');
        host = vec[0];
        port = vec[1];
    }
    // std::string type = "binary";
    router->cluster()->boss()->send2(router, client, "127.0.0.1:"+port+"/serve-fed-model");
    return "TICKET";
}

API(ServeFedModel, {})
    BMAG("SERVING FED MODEL...\n");
    if (router->needsAggregate()) {
        std::string bytes;
        if (router->federator()->round() == 0) { 
            bytes = Jericho::FileSystem::readBinary("./py/scripts/torch.pt");
        } else {
            bytes = router->federator()->bytes();
        }
        BYEL("HOST BYTES SIZE...? %li\n", bytes.size());
        std::string h = API_ARG(args, std::string("Host"));
        std::string host;
        std::string port;
        if (h != "undefined") {
            std::vector<std::string> vec = tokenize(h, ':');
            host = vec[0];
            port = vec[1];
        }
        std::string type = "binary";
        router->cluster()->boss()->send2(router, client, "127.0.0.1:"+port+"/train", "binary", bytes);
        return "TICKET";
    } else {
        return JsonResponse::error(500, "Federator not active!\n");
    }
}

API(Federation, {})
    BYEL("FEDERATING...\n");

    std::string numClients_ = API_ARG(args, std::string("num-clients"));
    std::string numRounds_ = API_ARG(args, std::string("num-rounds"));
    std::string timeout_ = API_ARG(args, std::string("timeout"));
    if (numClients_ == "undefined") numClients_ = "1";
    if (numRounds_ == "undefined") numRounds_ = "2";
    if (timeout_ == "undefined") timeout_ = "60";
    int timeout = std::stoi(timeout_);
    int numRounds = std::stoi(numRounds_);
    int numClients = std::stoi(numClients_);

    bool has_clients = false;
    if (router->needsAggregate()) {
        BGRE("Clients in cluster detected!\n");
        std::string bytes = Jericho::FileSystem::readBinary("./py/scripts/torch.pt");
        BYEL("HOST BYTES SIZE...? %li\n", bytes.size());
        std::string type = "binary";
        std::vector<ClusterQuorum*> newClients = router->cluster()->index()->selectType(FL_JOINED);
        std::vector<std::pair<std::string, std::string>> set;
        for (auto p : newClients) {
            set.push_back({p->host, p->port});
        }
        if (set.size() > 0) {
            has_clients = true;
            router->cluster()->index()->quorumTrain(newClients);
            router->cluster()->boss()->broadcastNaive(router, client, set, "/train", group_callback, type, bytes);
        }
    } else {
        BRED("No clients in cluster detected!\n");
    }

    router->federate(numClients, numRounds, timeout);
    if (has_clients) {
        return "TICKET";
    }
    return JsonResponse::success(200, "No clients in network");
}

API(ResetQuorum, {})
    router->cluster()->index()->resetQuorum();
    return JsonResponse::success(200, "Reset quorum");
}

API(Train, {}) 
    BYEL("TRAINING...\n");
    std::string content = args["content"];
    std::string path = router->cluster()->boss()->dir();
    if (router->cluster()->boss()->port() == "8090") {
        sleep(20);
    }
    if (router->cluster()->boss()->port() == "8089") {
        sleep(10);
    } 
    BWHI("PATH: %s\n", path.c_str());
    BWHI("CONTENT SIZE: %li\n", content.size());
    std::string dir = "./public/cluster/" + router->cluster()->boss()->port();
    path = "./public/cluster/" + router->cluster()->boss()->port() + "/torch.pt";
    BWHI("PATH 2: %s\n", path.data());
    Jericho::FileSystem::writeBinary(path.c_str(), content);
    std::string bytes = Jericho::FileSystem::readBinary(path.c_str());
    std::string bytes2 = Jericho::FileSystem::readBinary("./py/scripts/torch.pt");
    BBLU("BYTES TRANSFER...? %li\n", bytes.size());
    BBLU("BYTES ORIGINAL...? %li\n", bytes2.size());
    BGRE("ARE THEY EQUAL? %i\n", (int)(bytes == bytes2));
    router->train(true);
    return JsonResponse::success(200, "Model received");
}

API(JoinWeights, {})
    BYEL("JOINING WEIGHTS...\n");
    std::string content = args["content"];
    std::string h = API_ARG(args, std::string("Host"));
    std::string host;
    std::string port;
    if (h != "undefined") {
        std::vector<std::string> vec = tokenize(h, ':');
        host = vec[0];
        port = vec[1];
    }
    std::string path = router->cluster()->boss()->dir();
    BWHI("PATH: %s\n", path.c_str());
    BWHI("CONTENT SIZE: %li\n", content.size());
    std::string dir = "./public/cluster/aggregator/";
    path = dir + port + ".wt";
    BWHI("PATH 2: %s\n", path.data());
    Jericho::FileSystem::writeBinary(path.c_str(), content);
    std::string bytes = Jericho::FileSystem::readBinary(path.c_str());
    std::string verify = "./public/cluster/" + port + "/mnist_train.wt";
    std::string bytes2 = Jericho::FileSystem::readBinary(verify.c_str());
    BBLU("BYTES TRANSFER...? %li\n", bytes.size());
    BBLU("BYTES ORIGINAL...? %li\n", bytes2.size());
    BGRE("ARE THEY EQUAL? %i\n", (int)(bytes == bytes2));
    if (router->cluster()->index()->activateClient(host, port) < 0) {
        BRED("Failed to activate client: %s:%s\n", host.c_str(), port.c_str());
        BYEL("SIZE OF QUORUM IS: %li\n", router->cluster()->index()->quorum().size());
    }
    return JsonResponse::success(200, "Aggregator received weights\n");
}

    // method 4
    // ============================================================
    // int i = system(command_path.c_str());

    // sleep(10);

    // GRE("resource: %s\n", resource);
    // char* vals = strstr(resource, "\r\n\r\n");
    // if (vals == NULL) {
    //     BRED("SUBSTRING NOT FOUND!\n");
    //     exit(1);
    // }
    // printf("vals: %s\n", vals + 4);
    // int position = vals - resource;
    // printf("idx: %d\n", position);
    // char* valcopy = vals;
    // char* token;
    

    // method 2
    // =========================================================
    // int number, statval;
    // int child_pid;
    // printf("%d: I'm the parent !\n", getpid());
    // child_pid = fork();
    // if(child_pid == -1) { 
    //     printf("could not fork! \n");
    //     // exit( 1 );
    // } else if(child_pid == 0) {
    //     execl(command_path.c_str(), dir.c_str());
    // } else {
    //     printf("PID %d: waiting for child\n", getpid());
    //     waitpid( child_pid, &statval, WUNTRACED
    //                 #ifdef WCONTINUED       /* Not all implementations support this */
    //                         | WCONTINUED
    //                 #endif
    //                 );
    //     if(WIFEXITED(statval))
    //         printf("Child's exit code %d\n", WEXITSTATUS(statval));
    //     else
    //         printf("Child did not terminate with exit\n");
    // }

    // method 3
    // =========================================================
    // char* arg1 = &command_path[0];
    // char* arg2 = &dir[0];

    // printf("%s\n", arg1);
    // printf("%s\n", arg2);

    // int status;
    // char* paramsList[] = {
    //     "python3 ./py/server.py",
    //     "./public/cluster/8081",
    //     NULL
    // };

    // if ( fork() == 0 ){
    //     printf("I am the child\n");
    //     execv(paramsList[0],paramsList); 
    // } else {
    //     printf("I am the parent\n");
    //     wait( &status ); 
    // }

    // return 0;


    // return JsonResponse::success(200, result);

#endif