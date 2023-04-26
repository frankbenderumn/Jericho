#ifndef API_BIFROST_H_
#define API_BIFROST_H_

#include "api/api_helper.h"
#include "api/api.h"

API(AsyncRPC, {})
    REQUEST_INFO
    System* sys = router;
    BMAG("Firing RPC Call!\n");
    std::string url = "https://127.0.0.1:8081/async-job";
    int ticket = sys->bifrost()->send_async(url, "Let's get this bread!", NULL);
    sys->bifrost()->dumpBrokerSizes();
    return "TICKET";
}

API(AsyncJob, {})
    BMAG("Firing Async job!\n");
    REQUEST_INFO
    System* sys = router;
    std::string content = req->arg("content");
    BMAG("Job Test: %s\n", req->header("Host").c_str());
    MAG("Content: %s\n", content.c_str());
    std::string reply = req->reply("job", "/null");
    BYEL("reply: %s\n", reply.c_str());
    sys->bifrost()->reply(req, reply, "Bread received!", NULL);
    return "COMPLETE";
}

API(SyncRPC, {})
    REQUEST_INFO
    System* sys = router;
    BMAG("Firing RPC Call!\n");
    std::string url = "https://127.0.0.1:8081/sync-job";
    std::string result = sys->bifrost()->send(url, "Let's get this bread!", NULL);
    sys->bifrost()->dumpBrokerSizes();
    return result;
}

API(SyncJob, {})
    return "Asynchronous hello from " + router->bifrost()->hostname();
}

API(BroadcastAsyncRPC, {})
    REQUEST_INFO
    System* sys = router;
    BMAG("Firing RPC Call!\n");
    std::vector<std::string> urls = {
        "https://127.0.0.1:8081/broadcast-async-job",
        "https://127.0.0.1:8082/broadcast-async-job",
        "https://127.0.0.1:8083/broadcast-async-job",
        "https://127.0.0.1:8084/broadcast-async-job",
        "https://127.0.0.1:8085/broadcast-async-job",
        "https://127.0.0.1:8086/broadcast-async-job",
        "https://127.0.0.1:8087/broadcast-async-job",
        "https://127.0.0.1:8088/broadcast-async-job",
        "https://127.0.0.1:8089/broadcast-async-job",
        "https://127.0.0.1:8090/broadcast-async-job"
    };
    int ticket = sys->bifrost()->broadcast_async(urls, "Let's get this bread!", NULL);
    sys->bifrost()->dumpBrokerSizes();
    return "TICKET";
}

API(BroadcastAsyncJob, {})
    System* sys = router;
    std::string content = req->arg("content");
    BMAG("Job Test: %s\n", req->header("Host").c_str());
    MAG("Content: %s\n", content.c_str());
    std::string response = "Bread received from " + sys->bifrost()->hostname() + "!";
    std::string reply = req->reply("job", "/null");
    BYEL("reply: %s\n", reply.c_str());
    sys->bifrost()->reply(req, reply, response, NULL, "broadcast");
    return "COMPLETE";
}

API(BroadcastSyncRPC, {})
    REQUEST_INFO
    System* sys = router;
    BMAG("Firing RPC Call!\n");
    std::vector<std::string> urls = {
        "https://127.0.0.1:8081/broadcast-sync-job",
        "https://127.0.0.1:8082/broadcast-sync-job",
        "https://127.0.0.1:8083/broadcast-sync-job",
        "https://127.0.0.1:8084/broadcast-sync-job",
        "https://127.0.0.1:8085/broadcast-sync-job",
        "https://127.0.0.1:8086/broadcast-sync-job",
        "https://127.0.0.1:8087/broadcast-sync-job",
        "https://127.0.0.1:8088/broadcast-sync-job",
        "https://127.0.0.1:8089/broadcast-sync-job",
        "https://127.0.0.1:8090/broadcast-sync-job"
    };
    std::string result = sys->bifrost()->broadcast(urls, "Let's get this bread!", NULL);
    sys->bifrost()->dumpBrokerSizes();
    return result;
}

API(BroadcastSyncJob, {})
    return "Synchronous hello from " + router->bifrost()->hostname();
}

API(BinaryRPC, {})
    std::string s = "This is the end of the binary file";
    for (int i = 0; i < 100; i++) {
        s += '\0';
    }
    s += "This is the end of the binary file";
    std::string url = "bin+https://127.0.0.1:8081/binary-job";
    int ticket = router->bifrost()->send_async(url, s, NULL);
    return "TICKET";
}

API(BinaryJob, {})
    std::string test = "This is the end of the binary file";
    for (int i = 0; i < 100; i++) {
        test += '\0';
    }
    test += "This is the end of the binary file";
    std::string response;
    if (req->content == test) {
        response = "binary files match";
    } else {
        response = "binary files do not match";
    }
    std::string reply = req->reply("bin+job", "/null");
    router->bifrost()->reply(req, reply, response, NULL);
    return "COMPLETE";
}

API(CallbackRPC, {})
    return "TICKET";
}

API(Callback1Job, {})
    return "TICKET";
}

API(Callback2Job, {})
    return "TICKET";
}

API(PulseRPC, {})
    return "TICKET";
}

API(PulseJob, {})
    return "TICKET";
}

API(PulseFunctionRPC, {})
    return "TICKET";
}

API(PulseFunctionJob, {})
    return "TICKET";
}

API(FTP, {})
    REQUEST_INFO
    System* sys = router;
    BMAG("Firing RPC Call!\n");
    std::string url = "https://127.0.0.1:8081/bifrost-ping";
    std::string file = sys->bifrost()->send(url, "Let's get this bread!", NULL);
    sys->bifrost()->dumpBrokerSizes();
    return file;
}

#endif