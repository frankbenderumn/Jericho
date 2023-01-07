#ifndef ROUTER_ROUTES_H_ 
#define ROUTER_ROUTES_H_

#include "router/router.h"
#include "api/polygon.h"

void compile_routes(Router* router) {
    router->bind(ROUTE_API, "/rsi", apiRsi);
    router->bind(ROUTE_API, "/sma", apiSma);
    router->ipath("/jericho/app", "/jericho/app.iris");
    router->ipath("/jericho/user", "/jericho/user.iris");
    router->ipath("/celerity", "/celerity/index.html");
    router->bindSystem("/spawn", apiSpawn);
    router->bindSystem("/python", apiPython);
    router->bindSystem("/mongo-databases", apiMongoDatabases);
    router->bindSystem("/mongo-insert", apiMongoInsert);
    router->bindSystem("/federate", apiFederation);
    router->bindSystem("/federate-local", apiFederateLocal, ROUTE_RAW);
    router->bindSystem("/ping-one", apiPingOne);
    router->bindSystem("/ping-all", apiPingAll);
    router->bindSystem("/ping-local", apiPingLocal);
    router->bindSystem("/postgres", apiPostgres);
    router->bindSystem("/ping", apiPing);
    router->bindSystem("/echo", apiEcho);
    router->bind(ROUTE_API, "/model-json", apiServeModelJson);
    router->bindSystem("/model", apiServeModel, ROUTE_RAW);
    router->bindSystem("/new-model", apiNewModel);
    router->bindSystem("/request-join", apiRequestJoin);
    router->bindSystem("/dispatch-join", apiDispatchJoin);
    router->bindSystem("/join", apiJoin);
    router->secure("/jericho/denathrius.html");
    router->bindSystem("/train", apiTrain);
    router->bindSystem("/join-weights", apiJoinWeights);
    router->bindSystem("/get-fed-model", apiGetFedModel);
    router->bindSystem("/serve-fed-model", apiServeFedModel);
    router->bindSystem("/reset-quorum", apiResetQuorum);
    router->bindSystem("/fed-node", apiFedNode);
    router->bindSystem("/db-synch", apiDbSynch);
    router->bindSystem("/db-request-synch", apiDbRequestSynch);
    router->bindSystem("/setup", apiSetup);
    router->bindSystem("/save-script", apiSaveScript);
    router->bindSystem("/load-script", apiLoadScript);
    router->bindSystem("/list-scripts", apiListScripts);
    router->bindSystem("/update-federator", apiUpdateFederator);
    router->bindSystem("/reset-federator", apiResetFederator);

    // websocket functions
    router->bindSystem("/ws-alert", apiWsAlert);
    router->bindSystem("/ws-round", apiWsRound);
    router->bindSystem("/ws-accuracy", apiWsAccuracy);
    router->bindSystem("/ws-latency", apiWsLatency);
    router->bindSystem("/ws-bandwidth-in", apiWsBandwidthI);
    router->bindSystem("/ws-bandwidth-out", apiWsBandwidthO);
    router->bindSystem("/ws-bandwidth-in", apiWsBandwidthI);
    router->bindSystem("/ws-final-latency", apiWsFinalLatency);
    router->bindSystem("/ws-final-train", apiWsFinalTrain);
    router->bindSystem("/ws-final-rounds", apiWsFinalRounds);
    router->bindSystem("/ws-node-connect", apiWsNodeConnect);
    router->bindSystem("/ws-node-disconnect", apiWsNodeDisconnect);
    router->bindSystem("/ws-state", apiWsState);
    router->bindSystem("/ws-tolerant-state", apiWsTolerantState);

}

#endif