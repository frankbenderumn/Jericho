#ifndef API_DATABASE_H_
#define API_DATABASE_H_

#include "api/api.h"

API(MongoDatabases, {})
    if (!contains(TOKEN_LIST, args["token"])) {
        return JsonResponse::error(404, "Invalid token provided");
    }

    std::vector<std::string> result = router->celerity()->mongo()->databases();

    return JsonResponse::success(200, "Suceesfully return mongo databases");

}

API(MongoInsert, {})
    if (!contains(TOKEN_LIST, args["token"])) {
        return JsonResponse::error(404, "Invalid token provided");
    }

    std::vector<std::string> result = router->celerity()->mongo()->databases();

    return JsonResponse::success(200, "Suceesfully return mongo databases");

}

API(MongoCollection, {})
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
            return JsonResponse::error(200, "Placeholder");
            // return PolygonClient::rsi(v["ticker"], v["timespan"], v["adjusted"], v["window"], v["series_type"], "desc");
        } else {
            return JsonResponse::error(404, "Invalid token provided");
        }
    } else {
        print(keys(args));
        return JsonResponse::error(500, "invalid arguments provided");
    }
}

API(DbRequestSynch, {})
    BMAG("REQUESTING DB SYNCH...\n");
    if (client == NULL) {
        BRED("CLIENT IS NULL SOMEHOW :: apiDBRequestSynch\n");
    }
    MessageBuffer* buf = router->cluster()->buffer(client->url, "/db-synch");
    buf->port = "8080";
    router->cluster()->boss()->brokerSend(router, client->url, "/db-synch", buf);
    return "TICKET";
}

API(DbSynch, {})
    BMAG("SYNCHING DB...\n");

    return JsonResponse::success(200, "Synching db...\n");
}

API(Postgres, {})
    BMAG("SYNCHING DB...\n");

    return JsonResponse::success(200, "Synching db...\n");
}

#endif