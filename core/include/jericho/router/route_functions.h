#ifndef JERICHO_ROUTER_ROUTER_FUNCTIONS_H_
#define JERICHO_ROUTER_ROUTER_FUNCTIONS_H_

#include <unordered_map>
#include <string>
#include "server/thread_pool.h"

class Router;

typedef std::string (*SystemFunction)(std::unordered_map<std::string, std::string>, Router*, Client*, MessageBroker*);
typedef std::string (*RouteFunction)(std::unordered_map<std::string, std::string>);
typedef std::string (*ClusterFunction)(ThreadPool* tpool, void* (*worker)(void*), std::string message);

#endif