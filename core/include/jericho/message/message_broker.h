#ifndef JERICHO_API_MESSAGE_BROKER_H_
#define JERICHO_API_MESSAGE_BROKER_H_

#include <unordered_map>
#include <deque>
#include <utility>
#include <vector>

#include "message/message_buffer.h"

class ClusterNode;

enum BrokerType {
	BROKER_NULL,
	BROKER_BARRIER,
	BROKER_FIFO,
	BROKER_LIFO,
	BROKER_RR
};

// class MessageBuffer;

class MessageBroker {

	std::deque<MessageBuffer*> _messages;
	
	std::unordered_map<int, int> _tickets;
	
	std::unordered_map<int, MessageBuffer*> _promised;
	
	BrokerType _type = BROKER_NULL;
	
	MessageCallback _callback;
	
	int _epoch = 1;
	
	std::deque<MessageBuffer*> _stash;

  public:
	MessageBroker(BrokerType type, MessageCallback callback, int epoch = 1);

	~MessageBroker() {
		for (auto m : _messages) {
			delete m;
		}
	}

	void publishMessage(MessageBuffer* mbuf);

	void broadcast(Client* client, std::deque<MessageBuffer*> mq, std::vector<ClusterNode*> nodes);

	MessageCallback callback() const;

	bool hasMessages();

	bool hasTickets();

	bool ready(Client* client);

	std::deque<MessageBuffer*> response(Client* client);

	void markMessage(MessageBuffer* mbuf);

	int epoch();

	void epoch(int epoch);

	void refresh();

	void stash(std::deque<MessageBuffer*>);

	std::deque<MessageBuffer*> stashed();

};

#endif