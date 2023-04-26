#include "system/routes.h"
#include "api/apis.h"

void compile_routes(Router* router) {
#ifndef TEST_MODE

    // Routes
    router->ipath("/jericho/app", "/jericho/app.iris");
    router->ipath("/jericho/user", "/jericho/user.iris");
    router->ipath("/login-demo", "/staging/main.iris");
    router->secure("/staging/dashboard.iris");
    router->ipath("/celerity", "/celerity/index.html");
    router->resource("/user-login", oauth::login);
    router->secure("/jericho/denathrius.html");
    // router->mask("/web_manager.html", "/admin-dashboard");

    // Fin API
    // router->bind(ROUTE_API, "/rsi", apiRsi);
    // router->bind(ROUTE_API, "/sma", apiSma);

    // System module
    router->system("/spawn", apiSpawn);
    router->system("/python", apiPython);

    // Database Module
    router->system("/mongo-databases", apiMongoDatabases);
    router->system("/mongo-insert", apiMongoInsert);
    router->system("/postgres", apiPostgres);
    router->system("/db-synch", apiDbSynch);
    router->system("/db-request-synch", apiDbRequestSynch);

    // Ping Module
    router->system("/ping-one", apiPingOne);
    router->system("/ping-all", apiPingAll);
    router->system("/ping-local", apiPingLocal);
    router->system("/ping", apiPing);
    router->system("/echo", apiEcho);
    // router->bind(ROUTE_API, "/model-json", apiServeModelJson);

    // Bifrost Module
    router->system("/async-rpc", apiAsyncRPC);
    router->system("/async-job", apiAsyncJob);
    router->system("/sync-rpc", apiSyncRPC);
    router->system("/sync-job", apiSyncJob);
    router->system("/broadcast-sync-rpc", apiBroadcastSyncRPC);
    router->system("/broadcast-sync-job", apiBroadcastSyncJob);
    router->system("/broadcast-async-rpc", apiBroadcastAsyncRPC);
    router->system("/broadcast-async-job", apiBroadcastAsyncJob);
    router->system("/binary-rpc", apiBinaryRPC);
    router->system("/binary-job", apiBinaryJob);
    router->system("/callback-rpc", apiCallbackRPC);
    router->system("/callback-job1", apiCallback1Job);
    router->system("/callback-job2", apiCallback2Job);
    router->system("/pulse-rpc", apiPulseRPC);
    router->system("/pulse-job", apiPulseJob);
    router->system("/pulse-function-rpc", apiPulseFunctionRPC);
    router->system("/pulse-function-job", apiPulseFunctionJob);
    router->system("/ftp", apiFTP);

    router->system("/heartbeat", apiHeartbeat);

    // Federated Module
    router->system("/request-join", apiRequestJoin);
    router->system("/client-profile", apiClientProfile);
    router->system("/ping-ricochet", apiPingRicochet);
    router->system("/callback-resolution", apiCallbackResolution);
    router->system("/test-resolve", apiTestResolve);
    router->system("/train", apiTrain);
    router->system("/join-network", apiJoinNetwork);
    router->system("/aggregate-model", apiAggregateModel);
    router->system("/new-model", apiNewModel);

    // router->system("/federate-local", apiFederateLocal);
    // router->system("/model", apiServeModel);
    // router->system("/new-model", apiNewModel);
    // router->system("/dispatch-join", apiDispatchJoin);
    // router->system("/join", apiJoin);
    // router->system("/train", apiTrain);
    // router->system("/join-weights", apiJoinWeights);
    // router->system("/get-fed-model", apiGetFedModel);
    // router->system("/serve-fed-model", apiServeFedModel);
    // router->system("/reset-quorum", apiResetQuorum);
    // router->system("/fed-node", apiFedNode);
    // router->system("/update-federator", apiUpdateFederator);
    // router->system("/reset-federator", apiResetFederator);
    
    // Node Editor Module
    router->system("/setup", apiSetup);
    router->system("/save-script", apiSaveScript);
    router->system("/load-script", apiLoadScript);
    router->system("/list-scripts", apiListScripts);
    router->system("/launch-cluster", apiLaunchCluster);

    // Websocket Module
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