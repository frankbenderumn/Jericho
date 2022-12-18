#ifndef JERICHO_API_MESSAGE_BROKER_H_
#define JERICHO_API_MESSAGE_BROKER_H_

#include <unordered_map>
#include <deque>
#include <utility>
#include <vector>
#include <ctime>
#include <chrono>

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

typedef std::chrono::high_resolution_clock jclock;

class MessageBroker;

typedef std::unordered_map<std::string, std::string> Args;

typedef std::string (*__RPC)(Args, Router*, Client*, MessageBroker*);

enum BifrostInsType {
	BIN_NULL,
	BIN_RPC,
	BIN_REDUCE
};

typedef std::tuple<BifrostInsType, __RPC, std::string> Bins;

class MessageBroker {

	jclock::time_point t1 = jclock::now();

	std::deque<MessageBuffer*> _messages;
	
	std::unordered_map<int, int> _tickets;
	
	std::unordered_map<int, MessageBuffer*> _promised;
	
	BrokerType _type = BROKER_NULL;
	
	MessageCallback _callback;
	
	int _epoch = 0;
	int _epochs = 1;
	
	std::deque<MessageBuffer*> _stash;

	std::string _callbackType = "undefined";
    
	void* _args;

	bool _finished = false;

	std::unordered_map<int, Bins> _chain;
	// std::unordered_map<int, std::tuple<Args, Router*, Client*>> _chainArgs;

  public:
	MessageBroker(BrokerType type, MessageCallback callback, int epoch = 1);

	~MessageBroker() {
		for (auto m : _messages) {
			delete m;
		}
	}

	const bool timeout() const {
  		jclock::time_point t2 = jclock::now();
  		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		double secs = time_span.count();
		if (secs > 3) {
			return true;
		}
		return false;
	}

	void complete() { _finished = false; }

	const bool completed() const { return _finished; }

	void publishMessage(MessageBuffer* mbuf);

	void broadcast(std::string url, std::deque<MessageBuffer*> mq, std::vector<ClusterNode*> nodes);

	MessageCallback callback() const;

	bool hasMessages();

	bool hasTickets();

	bool ready(std::string url);

	std::deque<MessageBuffer*> response(std::string url);

	void markMessage(MessageBuffer* mbuf);

	const int epoch() const { return _epoch; }

	void epoch(int epoch) { _epoch = epoch; }

	const int epochs() { return _epochs; }

	void epochs(int epochs) { _epochs = epochs; }

	void refresh();

	void stash(std::deque<MessageBuffer*>);

	void args(void* args) { _args = args; }

	void* args() { return _args; }

	void callbackType(std::string callbackType) { _callbackType = callbackType; }

	std::string callbackType() { return _callbackType; }

	std::deque<MessageBuffer*> stashed();

	const std::string stringify() const {
		std::string result;
		for (auto m : _messages) {
			result += std::to_string(m->ticket) + "-" + m->received + "::";
		}
		if (result != "") {
			result.pop_back(); result.pop_back();
		}
		return result;
	}

	void chain(std::vector<__RPC> rpcs) {
		for (int i = _epochs + 1; i <= _epochs + rpcs.size(); i++) {
			_chain[i] = Bins{BIN_RPC, rpcs.at(i), "none"};
		}
		_epochs += rpcs.size();
	}

	void chain(__RPC rpc, std::string state = "none") {
		_epochs++;
		_chain[_epochs] = Bins(BIN_RPC, rpc, state);
	}

	std::string reduce();

	void chainReduce(std::string state) {
		_epochs++;
		_chain[_epochs] = Bins{BIN_REDUCE, NULL, state};
	}

	void batchChainUniform(__RPC rpc, int count, std::string state) {
		int ct = _epochs + 1;
		while (ct <= _epochs + count) {
			_chain[ct] = Bins{BIN_RPC, rpc, state};
			ct++;
		}
		_epochs += count;
	}

	__RPC chain() {
		return std::get<1>(_chain[_epoch]);
	}

	const bool isRPC() {
		if (std::get<1>(_chain[_epoch]) != NULL) {
			return true;
		}
		return false;
	}

};

#endif