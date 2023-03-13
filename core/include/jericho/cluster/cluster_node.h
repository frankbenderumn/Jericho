#ifndef JERICHO_DIST_CLUSTER_NODE_H_
#define JERICHO_DIST_CLUSTER_NODE_H_

#include <string>
#include <deque>
#include <vector>
#include <unordered_map>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "server/defs.h"
#include "server/thread_pool.h"
#include "util/iters.h"
#include "message/message_broker.h"
#include "message/message_buffer.h"

#include "cluster/cluster_edge.h"
#include "cluster/cluster_index.h"

static int CLUSTER_ID = -1;

class System;

enum ClusterNodeType {
	CLUSTER_NODE_NULL,
	CLUSTER_NODE_BOSS,
	CLUSTER_NODE_WORKER,
	CLUSTER_NODE_DB_RELATIONAL,
	CLUSTER_NODE_DB_OBJECT,
	CLUSTER_NODE_DB_BUCKET,
	CLUSTER_NODE_DB_CACHE,
	CLUSTER_NODE_DB_GRAPH,
	CLUSTER_NODE_DB_HYPERGRAPH
};

struct BifrostBurst {
	std::string url = "undefined";
	Client* client = nullptr;
	MessageBroker* broker = nullptr;
	BifrostBurst(std::string url, Client* client, MessageBroker* broker) {
		this->url = url;
		this->client = client;
		this->broker = broker;
	}
};

class ClusterNode {

    bool _status = false;

	ClusterNodeType _type = CLUSTER_NODE_NULL;

	ClusterEdge* _edge = nullptr;

	std::vector<std::string> _files;

	std::unordered_map<int, std::deque<MessageBroker*>> _fetchBrokers;

	int _id = -1;

	long long _timestamp = 0;

	std::unordered_map<std::string, std::deque<MessageBroker*>> _brokers;

	std::vector<BifrostBurst*> _bursts = {};

  protected:

	std::string _host = "undefined";

	std::string _port = "undefined";

	std::string _dir = "undefined";

	ClusterIndex* _index;

  public:

  	void send(System* router, std::string url, std::string path, MessageBuffer* buf);

	MessageBuffer* buffer(std::string url, std::string path);

	MessageBuffer* buffer2(std::string url, std::string path, std::string content);

	ClusterNode() {}

    ClusterNode(std::string host, std::string port, std::string dir, ClusterIndex* index);

	~ClusterNode();

	std::vector<BifrostBurst*> bursts();

	void addBurst(BifrostBurst* burst);

	std::unordered_map<std::string, std::deque<MessageBroker*>>& brokers() { return _brokers; }

	void brokerBroadcast(System* router, std::string url, std::deque<MessageBuffer*> mq, MessageCallback callback);

	void brokerSend(System* router, std::string url, std::string path, MessageBuffer* buf, std::string type = "", std::string content = "");

	void serveBroker(std::string client, MessageBroker* broker);

	void send2(System* router, std::string url, std::string path, std::string type = "", std::string content = "");

	MessageBroker* poll(std::string url);

	void broadcastNaive(System* router, std::string url, std::vector<std::pair<std::string, std::string>> pairs, std::string path, MessageCallback callback, std::string type = "", std::string content = "");

	void broadcast(System* router, std::string url, std::string path, MessageCallback callback, std::string type = "", std::string content = "");

	void pulse(System* router, std::string url, std::string path, MessageBroker* broker);
	
    void pingOne(System* router, std::string url, ClusterNode* dest);

	void pingAll(System* router, std::string url, std::vector<std::pair<std::string, std::string>> set = {});

	const int id() const;

	ClusterEdge* edges() const;

	const std::vector<ClusterNode*>& nodes();

	void addNode(ClusterNode* node);

	void addEdge(ClusterEdge* edge);

	ClusterNodeType type();

	void type(ClusterNodeType type);

	const std::string host() const;

	const std::string port() const;

	const std::string dir() const { return _dir; }

	const long timestamp() const;

	bool hasEdge(std::string host, std::string port);

	ClusterNode* getEdge(std::string host, std::string port);

	void federate(System* router, std::string url, std::string path, int epochs, int clients);

	void print();

	const std::string url() const { return _host + ":" + _port; }

	const std::string name() const { return _host + ":" + _port; }

	int exists(std::string pathname) {
		struct stat info;
		if( stat( pathname.c_str(), &info ) != 0 ) {
			printf( "cannot access %s\n", pathname.c_str() );
			return 0;
		} else if( info.st_mode & S_IFDIR ) {// S_ISDIR() doesn't exist on my windows 
			printf( "%s is a directory\n", pathname.c_str() );
			return 1;
		} else {
			printf( "%s is no directory\n", pathname.c_str() );
			return 2;
		}
	}


	void configDir(std::string role) {
		std::string cldir = "./public/cluster/" + _port;
		if (this->exists(cldir) == 1) {
			if (role == "agg" || role == "main") {
				cldir += "/aggregator";
				if (this->exists(cldir) == 0) {
					mkdir(cldir.c_str(), 0777);
				}
			}
		} else {
			mkdir(cldir.c_str(), 0777);
		}
	}

};

#endif