#ifndef JERICHO_ROUTER_GUI_H_
#define JERICHO_ROUTER_GUI_H_

#include <string>

#include "message/message.h"
#include "system/system.h"

static std::unordered_map<FLStatusType2, std::string> fltype_to_string_map = {
    {FL2_JOINED, "joined"},
    {FL2_TRAINING, "training"},
    {FL2_TRAINED, "trained"}
};

class GUI {
  public:
    static void state(System* sys, FLStatusType2 type) {
        std::string content;
        std::string gui;
        if (sys->federator()) {
            sys->federator()->state(type);
            content = sys->federator()->stateStr();
            gui = sys->federator()->gui();
        } else {
            content = fltype_to_string_map[type];
            gui = "127.0.0.1:8080";
        }
        int result = sys->bifrost()->send_async("https://" + gui + "/ws-state", content);
    }

    static void tolerantState(System* sys, Client* client, std::string url, std::string status) {
        // std::string name = router->cluster()->boss()->name();
        // picojson::object o;
        // o["url"] = picojson::value(url);
        // o["state"] = picojson::value(status);
        // picojson::value v(o);
        // Message* ws = new Message(name, router->federator()->gui() + "/ws-tolerant-state", router->cluster()->boss()->dir(), v.serialize());
        // router->cluster()->boss()->send_async(router, client->url, std::string("/null"), ws);
    }

    static void accuracy(System* sys, std::string content, bool parse = true) {
        std::string accuracy = "0.0";
        if (parse) {
            LEXES ls = prizm::regex_locs(content, std::regex("Accuracy\\s=\\s[0-9\\.]+"));
            if (ls.size() > 0) {
                accuracy = ls[0].first;
                BYEL("acc: %s\n", accuracy.c_str());
                std::vector<std::string> accTok = prizm::tokenize(accuracy, ' ');
                accuracy = accTok[2];
                BYEL("acc: %s\n", accuracy.c_str());
            }
        } else {
            accuracy = content;
                BBLU("acc: %s\n", accuracy.c_str());
        }
        std::string gui;
        if (sys->federator()) {
            gui = sys->federator()->gui();
        } else {
            gui = "127.0.0.1:8080";
        }
        int result = sys->bifrost()->send_async("https://" + gui + "/ws-accuracy", accuracy);

    }

    static void rounds(System* sys, std::string content) {
        std::string gui;
        if (sys->federator()) {
            gui = sys->federator()->gui();
        } else {
            gui = "127.0.0.1:8080";
        }
        int result = sys->bifrost()->send_async("https://" + gui + "/ws-rounds", content);
    }

    static void latency(System* sys, std::string content) {
        std::string gui;
        if (sys->federator()) {
            gui = sys->federator()->gui();
        } else {
            gui = "127.0.0.1:8080";
        }
        int result = sys->bifrost()->send_async("https://" + gui + "/ws-latency", content);
    }

    static void bandwidth(System* sys, std::string content, std::string direction) {
        std::string path = "/ws-bandwidth";
        path += "-" + direction;
        std::string gui;
        if (sys->federator()) {
            gui = sys->federator()->gui();
        } else {
            gui = "127.0.0.1:8080";
        }
        int result = sys->bifrost()->send_async("https://" + gui + path, "null");
    }

    static void finalLatency(System* sys) {
        std::string gui;
        if (sys->federator()) {
            gui = sys->federator()->gui();
        } else {
            gui = "127.0.0.1:8080";
        }
        int result = sys->bifrost()->send_async("https://" + gui + "/ws-latency", "null");
    }

    static void finalTrain(System* sys) {
        std::string gui;
        if (sys->federator()) {
            gui = sys->federator()->gui();
        } else {
            gui = "127.0.0.1:8080";
        }
        int result = sys->bifrost()->send_async("https://" + gui + "/ws-latency", "null");
    }

    static void finalRounds(System* sys) {
        std::string gui;
        if (sys->federator()) {
            gui = sys->federator()->gui();
        } else {
            gui = "127.0.0.1:8080";
        }
        int result = sys->bifrost()->send_async("https://" + gui + "/ws-latency", "null");
    }

    static void connect(System* sys, std::string url) {
        std::string gui;
        if (sys->federator()) {
            gui = sys->federator()->gui();
        } else {
            gui = "127.0.0.1:8080";
        }
        int result = sys->bifrost()->send_async("https://" + gui + "/ws-node-connect", url);
    }

    static void disconnect(System* sys, std::string url) {
        std::string gui;
        if (sys->federator()) {
            gui = sys->federator()->gui();
        } else {
            gui = "127.0.0.1:8080";
        }
        int result = sys->bifrost()->send_async("https://" + gui + "/ws-node-disconnect", url);
    }

    static void disconnectAll(System* sys) {
        std::string gui;
        if (sys->federator()) {
            gui = sys->federator()->gui();
        } else {
            gui = "127.0.0.1:8080";
        }
        int result = sys->bifrost()->send_async("https://" + gui + "/ws-disconnect-all", "null");
    }
};

#endif