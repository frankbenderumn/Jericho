#ifndef JERICHO_MESSAGE_CALLBACK_H_
#define JERICHO_MESSAGE_CALLBACK_H_

#include <deque>

#include "system/system.h"
#include "server/defs.h"
#include "message/message.h"
#include "util/trace.h"

std::string single_callback(System* node, Client* client, std::deque<Message*> mq, std::string type, void* args);

std::string group_callback(System* node, Client* client, std::deque<Message*> mq, std::string type, void* args);

std::string epoch_callback(System* router, Client* client, std::deque<Message*> mq, std::string type, void* args);

std::string bm_callback(System* router, Client* client, std::deque<Message*> mq, std::string type, void* args);

std::string chain_callback(System* router, Client* client, std::deque<Message*> mq, std::string type, void* args);

#endif