#ifndef JERICHO_API_WS_H_
#define JERICHO_API_WS_H_

#include "api/api.h"

API(WsAccuracy, {})
    std::string host = req->header("Host");
    picojson::object o;
    size_t ptr;
    std::string h, p;
    if ((ptr = host.find(":")) != std::string::npos) {
        h = host.substr(0, ptr);
        p = host.substr(ptr+1, host.size()-ptr-1);
    }
    o["host"] = picojson::value(h);
    o["port"] = picojson::value(p);
    o["hostname"] = picojson::value(host);
    o["accuracy"] = picojson::value(req->content);
    BWHI("WsAccuracy: Object created\n");
    std::string s = JsonResponse::ws(200, req->content.c_str(), "accuracy", &o);
    BWHI("WsAccuracy: Response: %s\n", s.c_str());
    BWHI("WsAccuracy: Start\n");
    router->ws_send(s.c_str());
    return "FNF";
}

API(WsLatency, {})
    std::string host = req->header("Host");
    picojson::object o;
    size_t ptr;
    std::string h, p;
    if ((ptr = host.find(":")) != std::string::npos) {
        h = host.substr(0, ptr);
        p = host.substr(ptr+1, host.size()-ptr-1);
    }
    o["host"] = picojson::value(h);
    o["port"] = picojson::value(p);
    o["hostname"] = picojson::value(host);
    o["latency"] = picojson::value(req->content);
    std::string s = JsonResponse::ws(200, req->content.c_str(), "latency", &o);
    router->ws_send(s.c_str());
    return "FNF";
}

API(WsRound, {})
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    picojson::object o;
    o["host"] = picojson::value(host);
    o["port"] = picojson::value(port);
    o["hostname"] = picojson::value(hostname);
    o["round"] = picojson::value(content);
    std::string s = JsonResponse::ws(200, content.c_str(), "round", &o);
    router->ws_send(s.c_str());
    return "FNF";
}

API(WsBandwidthI, {})
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    picojson::object o;
    o["host"] = picojson::value(host);
    o["port"] = picojson::value(port);
    o["hostname"] = picojson::value(hostname);
    o["bandwidth-in"] = picojson::value(content);
    std::string s = JsonResponse::ws(200, content.c_str(), "bandwidth-in", &o);
    router->ws_send(s.c_str());
    return "FNF";
}

API(WsBandwidthO, {})
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    picojson::object o;
    o["host"] = picojson::value(host);
    o["port"] = picojson::value(port);
    o["hostname"] = picojson::value(hostname);
    o["bandwidth-out"] = picojson::value(content);
    std::string s = JsonResponse::ws(200, content.c_str(), "bandwidth-out", &o);
    router->ws_send(s.c_str());
    return "FNF";
}

API(WsState, {})
    std::string host = req->header("Host");
    picojson::object o;
    size_t ptr;
    std::string h, p;
    if ((ptr = host.find(":")) != std::string::npos) {
        h = host.substr(0, ptr);
        p = host.substr(ptr+1, host.size()-ptr-1);
    }
    // std::string hostname_ = req->header("Host");
    // size_t p = 
    o["host"] = picojson::value(h);
    o["port"] = picojson::value(p);
    o["hostname"] = picojson::value(host);
    o["state"] = picojson::value(req->content);
    std::string s = JsonResponse::ws(200, req->content.c_str(), "state", &o);
    BGRE("WsState: Sending websocket... %s\n", s.c_str());
    router->ws_send(s.c_str());
    return "HTTP/1.1 200 OK";
}

API(WsTolerantState, {})
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    picojson::value data;
    std::string cerr = picojson::parse(data, content);
    if (!cerr.empty()) {
        BRED("WsTolerantState: json parse failed\n");
    } else {
        picojson::object co = data.get<picojson::object>();
        std::string state = co["state"].get<std::string>();
        std::string url = co["url"].get<std::string>();
        picojson::object o;
        o["host"] = picojson::value(host);
        o["port"] = picojson::value(port);
        o["hostname"] = picojson::value(hostname);
        o["state"] = picojson::value(state);
        o["url"] = picojson::value(url);
        std::string s = JsonResponse::ws(200, "Undefined - WSTolerantState", "tolerant-state", &o);
        router->ws_send(s.c_str());
    }
    return "HTTP/1.1 200 OK";
}

API(WsNodeConnect, {})
    std::string host = req->header("Host");
    picojson::object o;
    size_t ptr;
    std::string h, p;
    if ((ptr = host.find(":")) != std::string::npos) {
        h = host.substr(0, ptr);
        p = host.substr(ptr+1, host.size()-ptr-1);
    }
    // std::string hostname_ = req->header("Host");
    // size_t p = 
    o["host"] = picojson::value(h);
    o["port"] = picojson::value(p);
    o["hostname"] = picojson::value(host);
    o["dest"] = picojson::value(req->content);
    std::string s = JsonResponse::ws(200, req->content.c_str(), "node-connect", &o);
    router->ws_send(s.c_str());
    return "FNF";
}

API(WsNodeDisconnect, {})
    std::string host = req->header("Host");
    picojson::object o;
    size_t ptr;
    std::string h, p;
    if ((ptr = host.find(":")) != std::string::npos) {
        h = host.substr(0, ptr);
        p = host.substr(ptr+1, host.size()-ptr-1);
    }
    // std::string hostname_ = req->header("Host");
    // size_t p = 
    o["host"] = picojson::value(h);
    o["port"] = picojson::value(p);
    o["hostname"] = picojson::value(host);
    o["dest"] = picojson::value(req->content);
    std::string s = JsonResponse::ws(200, req->content.c_str(), "node-disconnect", &o);
    router->ws_send(s.c_str());
    return "FNF";
}

API(WsFinalTrain, {})
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    picojson::object o;
    o["host"] = picojson::value(host);
    o["port"] = picojson::value(port);
    o["hostname"] = picojson::value(hostname);
    o["train"] = picojson::value(content);
    std::string s = JsonResponse::ws(200, content.c_str(), "final-train", &o);
    router->ws_send(s.c_str());
    return "FNF";
}

API(WsFinalLatency, {})
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    picojson::object o;
    o["host"] = picojson::value(host);
    o["port"] = picojson::value(port);
    o["hostname"] = picojson::value(hostname);
    o["latency"] = picojson::value(content);
    std::string s = JsonResponse::ws(200, content.c_str(), "final-latency", &o);
    router->ws_send(s.c_str());
    return "FNF";
}

API(WsFinalRounds, {})
    REQUEST_INFO
    std::string content = API_ARG(args, std::string("content"));
    picojson::object o;
    o["host"] = picojson::value(host);
    o["port"] = picojson::value(port);
    o["hostname"] = picojson::value(hostname);
    o["rounds"] = picojson::value(content);
    std::string s = JsonResponse::ws(200, content.c_str(), "final-rounds", &o);
    router->ws_send(s.c_str());
    return "FNF";
}

API(WsAlert, {})
    return "FNF";
}

API(WsNotification, {})
    return "FNF";
}

#endif