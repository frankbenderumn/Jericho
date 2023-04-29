#ifndef JERICHO_ROUTER_GUI_H_
#define JERICHO_ROUTER_GUI_H_

#include <string>

#include "message/message.h"
#include "system/system.h"

class GUI {
  public:
    static void state(System* router, Client* client) {
        // std::string name = router->cluster()->boss()->name();
        // std::string content = router->federator()->local()->stateStr();
        // Message* ws = new Message(name, router->federator()->gui() + "/ws-state", router->cluster()->boss()->dir(), content);
        // router->cluster()->boss()->send(router, client->url, std::string("/null"), ws);
    }

    static void tolerantState(System* router, Client* client, std::string url, std::string status) {
        // std::string name = router->cluster()->boss()->name();
        // picojson::object o;
        // o["url"] = picojson::value(url);
        // o["state"] = picojson::value(status);
        // picojson::value v(o);
        // Message* ws = new Message(name, router->federator()->gui() + "/ws-tolerant-state", router->cluster()->boss()->dir(), v.serialize());
        // router->cluster()->boss()->send(router, client->url, std::string("/null"), ws);
    }

    static void accuracy(System* router, Client* client, std::string content, bool parse = true) {
        // std::string accuracy = "0.0";
        // if (parse) {
        //     LEXES ls = prizm::regex_locs(content, std::regex("Accuracy\\s=\\s[0-9\\.]+"));
        //     if (ls.size() > 0) {
        //         accuracy = ls[0].first;
        //         BYEL("acc: %s\n", accuracy.c_str());
        //         std::vector<std::string> accTok = prizm::tokenize(accuracy, ' ');
        //         accuracy = accTok[2];
        //         BYEL("acc: %s\n", accuracy.c_str());
        //     }
        // } else {
        //     accuracy = content;
        //         BBLU("acc: %s\n", accuracy.c_str());
        // }
        // std::string name = router->cluster()->boss()->name();
        // Message* ws = new Message(name, router->federator()->gui() + "/ws-accuracy", router->cluster()->boss()->dir(), accuracy);
        // router->cluster()->boss()->send(router, client->url, std::string("/null"), ws);
    }

    static void rounds(System* router, Client* client, std::string content) {
        // std::string name = router->cluster()->boss()->name();
        // Message* ws = new Message(name, router->federator()->gui() + "/ws-rounds", router->cluster()->boss()->dir(), content);
        // router->cluster()->boss()->send(router, client->url, std::string("/null"), ws);
    }

    static void latency(System* router, Client* client, std::string content) {
        // std::string name = router->cluster()->boss()->name();
        // Message* ws = new Message(name, router->federator()->gui() + "/ws-latency", router->cluster()->boss()->dir(), content);
        // router->cluster()->boss()->send(router, client->url, std::string("/null"), ws);
    }

    static void bandwidth(System* router, Client* client, std::string content, std::string direction) {
        // std::string name = router->cluster()->boss()->name();
        // std::string path = "/ws-bandwidth";
        // path += "-" + direction;
        // Message* ws = new Message(name, router->federator()->gui() + path, router->cluster()->boss()->dir(), content);
        // router->cluster()->boss()->send(router, client->url, std::string("/null"), ws);
    }

    static void finalLatency(System* router, Client* client) {
        // std::string name = router->cluster()->boss()->name();
        // std::string content = std::to_string(router->federator()->finalLatency());
        // Message* ws = new Message(name, router->federator()->gui() + "/ws-final-latency", router->cluster()->boss()->dir(), content);
        // router->cluster()->boss()->send(router, client->url, std::string("/null"), ws);
    }

    static void finalTrain(System* router, Client* client) {
        // std::string name = router->cluster()->boss()->name();
        // std::string content = std::to_string(router->federator()->finalTrain());
        // Message* ws = new Message(name, router->federator()->gui() + "/ws-final-rounds", router->cluster()->boss()->dir(), content);
        // router->cluster()->boss()->send(router, client->url, std::string("/null"), ws);
    }

    static void finalRounds(System* router, Client* client) {
        // std::string name = router->cluster()->boss()->name();
        // std::string content = std::to_string(router->federator()->finalRounds());
        // Message* ws = new Message(name, router->federator()->gui() + "/ws-final-rounds", router->cluster()->boss()->dir(), content);
        // router->cluster()->boss()->send(router, client->url, std::string("/null"), ws);
    }

    static void connect(System* router, Client* client, std::string url) {
        // std::string name = router->cluster()->boss()->name();
        // Message* ws = new Message(name, router->federator()->gui() + "/ws-node-connect", router->cluster()->boss()->dir(), url);
        // router->cluster()->boss()->send(router, client->url, std::string("/null"), ws);
    }

    static void disconnect(System* router, Client* client, std::string url) {
        // std::string name = router->cluster()->boss()->name();
        // Message* ws = new Message(name, router->federator()->gui() + "/ws-node-disconnect", router->cluster()->boss()->dir(), url);
        // router->cluster()->boss()->send(router, client->url, std::string("/null"), ws);
    }

    static void disconnectAll(System* router, Client* client) {
        // std::string name = router->cluster()->boss()->name();
        // for (auto out : router->federator()->outs()) {
        //     Message* ws = new Message(name, router->federator()->gui() + "/ws-node-disconnect", router->cluster()->boss()->dir(), out);
        //     router->cluster()->boss()->send(router, client->url, std::string("/null"), ws);
        // }
    }
};

#endif