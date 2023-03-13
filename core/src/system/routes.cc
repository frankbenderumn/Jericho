#include "system/routes.h"
#include "api/apis.h"

void compile_routes(Router* router) {
#ifndef TEST_MODE
    router->bind(ROUTE_API, "/rsi", apiRsi);
    router->bind(ROUTE_API, "/sma", apiSma);
    router->ipath("/jericho/app", "/jericho/app.iris");
    router->ipath("/jericho/user", "/jericho/user.iris");

    router->ipath("/login-demo", "/staging/main.iris");
    router->secure("/staging/dashboard.iris");

    router->resource("/user-login", oauth::login);
    
    router->ipath("/celerity", "/celerity/index.html");
    router->system("/spawn", apiSpawn);
    router->system("/python", apiPython);
    router->system("/mongo-databases", apiMongoDatabases);
    router->system("/mongo-insert", apiMongoInsert);
    router->system("/federate", apiFederation);
    router->system("/federate-local", apiFederateLocal);
    router->system("/ping-one", apiPingOne);
    router->system("/ping-all", apiPingAll);
    router->system("/ping-local", apiPingLocal);
    router->system("/postgres", apiPostgres);
    router->system("/ping", apiPing);
    router->system("/echo", apiEcho);
    router->bind(ROUTE_API, "/model-json", apiServeModelJson);
    router->system("/model", apiServeModel);
    router->system("/new-model", apiNewModel);
    router->system("/request-join", apiRequestJoin);
    router->system("/dispatch-join", apiDispatchJoin);
    router->system("/join", apiJoin);

    router->secure("/jericho/denathrius.html");
    
    router->system("/train", apiTrain);
    router->system("/join-weights", apiJoinWeights);
    router->system("/get-fed-model", apiGetFedModel);
    router->system("/serve-fed-model", apiServeFedModel);
    router->system("/reset-quorum", apiResetQuorum);
    router->system("/fed-node", apiFedNode);
    router->system("/db-synch", apiDbSynch);
    router->system("/db-request-synch", apiDbRequestSynch);
    router->system("/setup", apiSetup);
    router->system("/save-script", apiSaveScript);
    router->system("/load-script", apiLoadScript);
    router->system("/list-scripts", apiListScripts);
    router->system("/update-federator", apiUpdateFederator);
    router->system("/reset-federator", apiResetFederator);

    // websocket functions
    router->system("/ws-alert", apiWsAlert);
    router->system("/ws-round", apiWsRound);
    router->system("/ws-accuracy", apiWsAccuracy);
    router->system("/ws-latency", apiWsLatency);
    router->system("/ws-bandwidth-in", apiWsBandwidthI);
    router->system("/ws-bandwidth-out", apiWsBandwidthO);
    router->system("/ws-bandwidth-in", apiWsBandwidthI);
    router->system("/ws-final-latency", apiWsFinalLatency);
    router->system("/ws-final-train", apiWsFinalTrain);
    router->system("/ws-final-rounds", apiWsFinalRounds);
    router->system("/ws-node-connect", apiWsNodeConnect);
    router->system("/ws-node-disconnect", apiWsNodeDisconnect);
    router->system("/ws-state", apiWsState);
    router->system("/ws-tolerant-state", apiWsTolerantState);
#endif
}