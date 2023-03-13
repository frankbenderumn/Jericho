if (router->flash()) {
    BYEL("FLASHING...\n");
    router->cluster()->boss()->brokerBroadcast(router, NULL, router->flashBuffer(), group_callback);
}

while (1) {
    MessageBroker* broker = router->cluster()->boss()->poll(NULL);
    if (broker != NULL) {
        BGRE("FIRING ASYNC CALL\n");
        std::string response = broker->callback()(router, NULL, broker->response(NULL), broker->callbackType(), NULL);
        if (broker->epoch() == 0) {
            // if (isHTTP(response)) {
            // 	resource::serve_raw(client, clients, response.c_str());
            // } else {
            // 	resource::serve_http(client, clients, response.c_str());
            // }
            // client->promised = false;
            // BBLU("I DONT UNDERSTAND: %s\n", response.c_str());
            // drop_client(client, clients); // this segfaults buffer size of 500000 but not 4095 (fixed)
            break;
        }
    }
}