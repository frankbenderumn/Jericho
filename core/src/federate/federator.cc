#include "federate/federator.h"    
#include "router/router.h"
#include "api/api_helper.h"
    
void Federator::poll(std::string hostname, Router* router, std::string url, MessageCallback callback, Benchmark* bm) {
    // if (router->needsAggregate()) {
    //     // long long t = std::time(NULL) - router->federator()->startTime() - router->federator()->waitTime();
    //     // BWHI("TIME TILL TIMEOUT: %lld\n", t * -1);
    //     if (std::time(NULL) - router->federator()->startTime() >= router->federator()->waitTime()) {
    //         BRED("FEDERATOR TIME LIMIT REACHED FOR AGGREGATION\n");
    //         router->cluster()->index()->quorumDrop();
    //         router->shutdownFederator();
    //     }

    //     if (router->federator() != nullptr) {
    //         std::vector<ClusterQuorum*> newClients = router->cluster()->index()->selectType(FL_JOINED);
    //         std::vector<ClusterQuorum*> joinedClients = router->cluster()->index()->selectType(FL_DELIVERED);
    //         std::vector<ClusterQuorum*> dormantClients = router->cluster()->index()->selectType(FL_DORMANT);
    //         std::vector<ClusterQuorum*> droppedClients = router->cluster()->index()->selectType(FL_DROPPED);
    //         std::vector<ClusterQuorum*> trainingClients = router->cluster()->index()->selectType(FL_TRAINING);
    //         BYEL("NEEDED: %i, JOINED: %li, TRAINING: %li, DELIVERED: %li, DORMANT: %li, DROPPED: %li\n", router->federator()->clients(), newClients.size(), trainingClients.size(), joinedClients.size(), dormantClients.size(), droppedClients.size());
    //         std::vector<std::pair<std::string, std::string>> set;
    //         for (auto p : newClients) {
    //             set.push_back({p->host, p->port});
    //         }

    //         if (set.size() > 0) {
    //             if (router->federator()->bytes() != "undefined") {
    //                 router->cluster()->boss()->broadcastNaive(router, url, set, "/train", callback, "binary", router->federator()->bytes());
    //             }
    //         }

    //         if (router->cluster()->index()->quorumMet(router->federator()->clients().size())) {
    //             BGRE("QUORUM MET, START AGGREGATION\n");
    //             std::vector<std::string> selection = router->cluster()->index()->quorumSelect(router->federator()->clients().size());
    //             if (selection.size() != 0) {
    //                 std::string ports = "[";
    //                 for (auto s : selection) {
    //                     ports += s + ",";
    //                 }
    //                 ports.pop_back(); ports += "]";
    //                 BBLU(" PORTS: %s\n", ports.c_str());
    //                 std::string command = "python3 ./py/fusion.py " + ports + " " + std::to_string(router->federator()->round()) + " " + std::to_string(router->federator()->id());
    //                 std::string results = pipe(command);
    //                 BWHI("AND?\n");
    //                 router->cluster()->index()->quorumUpdate(selection);
    //                 router->federator()->finishRound();
    //                 if (router->federator()->active()) {
    //                     std::string path = "./public/aggregator/aggregate-" + std::to_string(router->federator()->round() - 1) + ".pt"; 
    //                     std::string bytes = Jericho::FileSystem::readBinary(path.c_str());
    //                     std::vector<std::string> nextSelection = router->cluster()->index()->quorumSelect(router->federator()->clients().size());
    //                     std::vector<std::pair<std::string, std::string>> set;
    //                     for (auto p : nextSelection) {
    //                         set.push_back({"127.0.0.1", p});
    //                     }
    //                     router->federator()->bytes(bytes);
    //                 }
    //             } else {
    //                 BYEL("STOPPING FEDERATION DUE TO ERROR\n");
    //                 router->shutdownFederator();
    //             }
    //         }
    //     }
    // }
}

void Federator::train(std::string hostname, Router* router, std::string url, MessageCallback callback, Benchmark* bm) {
    while (router->needsTrain()) {
        if (router->needsTrain()) {
            BMAG("NEED TRAINING\n");
            std::string dir = "./public/cluster/" + router->cluster()->boss()->port();
            std::string command_path = "python3 ./py/torch_load.py " + dir;
            std::string results = pipe(command_path);
            router->train(false);
            std::string wts = dir + "/mnist_train.wt";
            std::string bytes = Jericho::FileSystem::readBinary(wts.c_str());
            router->cluster()->boss()->send2(router, url, "127.0.0.1:8080/join-weights", "binary", bytes);
        }
    }
}