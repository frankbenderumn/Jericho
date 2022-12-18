#ifndef JERICHO_MESSAGE_CALLBACK_H_
#define JERICHO_MESSAGE_CALLBACK_H_

#include <deque>

#include "router/router.h"
#include "server/defs.h"
#include "message/message_buffer.h"

std::string single_callback(Router* node, Client* client, std::deque<MessageBuffer*> mq, std::string type, void* args);

std::string group_callback(Router* node, Client* client, std::deque<MessageBuffer*> mq, std::string type, void* args);

std::string epoch_callback(Router* router, Client* client, std::deque<MessageBuffer*> mq, std::string type, void* args);

std::string bm_callback(Router* router, Client* client, std::deque<MessageBuffer*> mq, std::string type, void* args);

std::string chain_callback(Router* router, Client* client, std::deque<MessageBuffer*> mq, std::string type, void* args);

#endif