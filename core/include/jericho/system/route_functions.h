#ifndef JERICHO_ROUTER_ROUTER_FUNCTIONS_H_
#define JERICHO_ROUTER_ROUTER_FUNCTIONS_H_

#include <unordered_map>
#include <string>
#include "server/thread_pool.h"
#include "controller/controllers.h"
#include "server/response.h"
#include "server/request.h"
#include "session/session.h"

class System;

typedef std::string (*SystemFunction)(Request*, System*, Client*, MessageBroker*);
typedef std::string (*RouteFunction)(Request*, System*, Client*, MessageBroker*);
typedef Response* (*ResourceFunction)(System* system, Request* req, jericho::Session* sesh);
typedef int (*ContractFunction)(std::unordered_map<std::string, std::string>);
typedef std::string (*ClusterFunction)(ThreadPool* tpool, void* (*worker)(void*), std::string message);

#endif