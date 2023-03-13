#include "message/bifrost.h"
#include "system/system.h"
#include "server/request.h"
#include "server/fetch.h"
#include "celerity/celerity.h"
#include "message/message_buffer.h"
#include "message/message_broker.h"
#include "api/api_helper.h"
#include "message/callback.h"
#include "cluster/cluster_node.h"
#include "session/cookie.h"
#include "session/session.h"
#include "server/defs.h"
#include "server/client.h"

// for client only use, may need to make thread pool more robust to handle behavior that doesn't require incoming connections

void Bifrost::burst(System* router) {
    MAG("Bifrost::serve: Start...\n");
    std::vector<BifrostBurst*> bursts = router->cluster()->boss()->bursts();
    MAG("Bifrost::serve: Burst size: %li\n", bursts.size());
    if (bursts.size() > 0) {
        for (auto burst : bursts) {
            MessageBroker* broker = burst->broker;
            if (broker->hasMessages()) {
                MAG("Bifrost::serve: FIRING ASYNC CALL - EPOCH: %i\n", broker->epoch());
                MAG("Bifrost::serve: Client url is: %s\n", burst->url.c_str());
                MAG("Bifrost::serve: Compiling callback...\n");
                std::deque<MessageBuffer*> bufs = broker->response(burst->url);
                broker->stash(bufs);
                std::string response = broker->callback()(router, burst->client, bufs, broker->callbackType(), broker->args());
                MAG("Bifrost::serve: Response: %s\n", response.c_str());    
                if (broker->epoch() == broker->epochs()) {
                    if (isHTTP(response)) {
                        MAG("Bifrost::serve: Serve raw\n");
                        // resource::serve_raw(burst->client, clients, response.c_str());
                    } else {
                        MAG("Bifrost::serve: Serve HTTP\n");
                        // resource::serve_http(burst->client, clients, response.c_str());
                    }
                    // client->promised = false;
                    router->cluster()->boss()->serveBroker(burst->url, broker);
                    MAG("Bifrost::serve: End poll true\n");
                    // drop_client(client, clients); // this segfaults buffer size of 500000 but not 4095 (fixed...?)
                }
            }
        }
    }
}

bool Bifrost::poll(System* router, std::string url, Client* client, Client** clients) {
    // MAG("Bifrost::poll: They see me pollin, they hatin\n");
    MessageBroker* broker = router->cluster()->boss()->poll(url);
    if (broker != NULL) {
        MAG("Bifrost::poll: FIRING ASYNC CALL - EPOCH: %i\n", broker->epoch());
        MAG("Bifrost::poll: Client url is: %s\n", url.c_str());
        MAG("Bifrost::poll: Compiling callback...\n");
        std::deque<MessageBuffer*> bufs = broker->response(url);
        broker->stash(bufs);
        std::string response = broker->callback()(router, client, bufs, broker->callbackType(), broker->args());
        MAG("Bifrost::poll: Response: %s\n", response.c_str());    
        MAG("Bifrost::poll: Epoch: %i, Epochs: %i\n", broker->epoch(), broker->epochs());    
        if (broker->epoch() - 1 == broker->epochs()) {
            if (isHTTP(response)) {
                MAG("Bifrost::poll: Serve raw\n");
                resource::serve_raw(client, clients, response.c_str());
            } else {
                MAG("Bifrost::poll: Serve HTTP\n");
                resource::serve_http(client, clients, response.c_str());
            }
            // client->promised = false;
            router->cluster()->boss()->serveBroker(url, broker);
            MAG("Bifrost::poll: End poll true\n");
            return true;
            // drop_client(client, clients); // this segfaults buffer size of 500000 but not 4095 (fixed...?)
        }
    }
    // MAG("Bifrost::poll: End poll false\n");
    return false;
}

bool authenticate(System* sys, Request* req) {
	bool username = false;
	bool password = false;
	if (req->arg("username") == "joey") { username = true; }
	if (req->arg("password") == "pass1234") { password = true; }
	if (username && password) {
		return true;
	}
	return false;
}

SessionManager* Bifrost::sessionManager() const { return _sm; }

void Bifrost::serve(System* sys, Client* client, Client** clients, Request* req) {
    std::string result;
    std::string ipath;
    Route* route;
    Response* resp;
    Cookie* cookie = 0;

    std::string cookie_header = req->header("Cookie");

    if (cookie_header != "undefined") {
        BGRE("Cookie detected!\n");
        std::vector<std::string> toks = prizm::tokenize(cookie_header, "; ");
        if (toks.size() > 1) {
            for (auto t : toks) {
                BYEL("Cookie: %s\n", t.c_str());
            }
        } else {
            BYEL("Cookie: %s\n", cookie_header.c_str());
            std::string::size_type p;
            if ((p = cookie_header.find("-")) != std::string::npos) {
                std::string prefix = cookie_header.substr(0, p);
                std::string cook = cookie_header.substr(p+1, cookie_header.size() - p - 1);
                BWHI("Cook: %s\n", cook.c_str());
                if ((p = cook.find("=")) != std::string::npos) {
                    std::string key = cook.substr(0, p);
                    std::string val = cook.substr(p+1, cook.size() - p - 1);
                    BYEL("Key: %s, Val: %s\n", key.c_str(), val.c_str());
                    std::string addr = client_get_address(client);
                    cookie = new Cookie(addr, key, val);
                }

                if (prefix == "__Host") {
                    BBLU("Host prefix detected!\n");
                } else if (prefix == "__Secure") {
                    BBLU("Secure prefix detected!\n");
                }
            }
        }
    }

    printf("Cookie verification...");
    jericho::Session* sesh = 0;
    if (cookie) {
        sesh = this->_sm->verify(cookie);
    }
    
    if (sesh) {
        BGRE(" Session found!\n");
    } else {
        BRED(" Session not found!\n");
    }

//     oewn-01590626-v (Interlingual Index: i29667)
// (v) bank-bank%2.35.00.. enclose with a bank “bank roads”
// Topic: verb.contact
// Subcategorization Frames:
// Somebody banks something
// MORE ▶

    switch (sys->router()->protocol(req->path)) {
        case ROUTE_RAW:
            result = sys->router()->exec(ROUTE_RAW, req->path, req->args, sys, client);
            resource::serve_http(client, clients, result.c_str());	
            break;									
        case ROUTE_SYSTEM:
            BYEL("System...\n");
            result = sys->router()->exec(ROUTE_SYSTEM, req->path, req->args, sys, client);
            if (result == "TICKET") {
                GRE("Bifrost::serve: Ticket Recevied -- will not drop client for now\n");
                req->async = true;
            } else {
                GRE("Bifrost::serve: System call sending http result: %.100s\n", result.c_str());
                resource::serve_http(client, clients, result.c_str());		
            }
            break;
        case ROUTE_API:
            BYEL("API CALL...\n");
            result = sys->router()->exec(ROUTE_API, req->path, req->args, sys, client);
            GRE("Bifrost::serve: API call sending http result: %.100s\n", result.c_str());
            resource::serve_http(client, clients, result.c_str(), std::string("application/json"));
            break;
        case ROUTE_RESOURCE:
            BYEL("RESOURCE CALL...\n");
            // resp = sys->router()->resource(sys, req);
            resp = sys->router()->resource(sys, req, sesh);
            if (resp) {
                resp->serve();
            } else {
                BRED("Response is NULL\n");
                resource::error(sys->router(), client, "305");
            }
            break;
        case ROUTE_SECURE:
            BYEL("SECURE CALL...\n");
            if (sesh == NULL) {
                BRED("Bifrost::serve: No active session. Redirecting to login...\n");
                resource::error(sys->router(), client, "305");    
            }

            route = sys->router()->route(req->path);

            if (route) {
            // if (authenticate(sys, req)) {
                BGRE("Serving secure route...!\n");
                if (route->callback) {
                    result = route->callback(req->args);
                } else {
                    BRED("No callback detected!\n");
                    PLOG(LSERVER, "\033[1;32mSecure\033[0m route being served.");
                    result = sys->router()->exec(ROUTE_HTTP, req->path, req->args, sys, client);
                    resource::serve_cxx(sys, client, clients, req->path.c_str());
                }
            } else {
                // add a redirect
                resource::error(sys->router(), client, "305");
            }
            break;
        case ROUTE_IRIS:
            BYEL("IRIS CALL...\n");
            ipath = sys->router()->ipath(req->path);
            resource::serve_cxx(sys, client, clients, ipath.c_str());
            BGRE("Iris call done!\n");
            // drop_client(client, clients, &router->num_clients);
            break;
        case ROUTE_NULL:
        case ROUTE_HTTP:
            // BMAG("Bifrost::serve: Http or Null route!\n");
            // if (sys->router()->secured(req->path)) {
            //     // BYEL("Bifrost::serve: Route is sensitive!\n");
            //     if (authenticate(sys, req)) {
            //         // BGRE("Serving secure route...!\n");
            //         PLOG(LSERVER, "\033[1;32mSecure\033[0m route being served.");
            //         result = sys->router()->exec(ROUTE_HTTP, req->path, req->args, sys, client);
            //         resource::serve_cxx(sys, client, clients, req->path.c_str());
            //     } else {
            //         resource::error(sys, client, "305");
            //     }
            // } else {
                // MAG("Bifrost::serve: Unsecure null/http route. (Default?)\n");
                result = sys->router()->exec(ROUTE_HTTP, req->path, req->args, sys, client);
                // MAG("Bifrost::serve: Route exec result is:\n\t%s\n", result.c_str());
                resource::serve_cxx(sys, client, clients, req->path.c_str());
                // CYA("Interpetted Request\n=============================\n");
                // CYA("Path: %s\n", req->path.c_str());
                // CYA("%s\n", req->content.c_str());
                // drop_client(client, clients, &router->num_clients);
            // }
            break;
        default:
            BRED("UNREGISTERED ROUTE\n");
            resource::serve_http(client, clients, "Route does not exist", std::string("application/json"));
            break;
    }
}