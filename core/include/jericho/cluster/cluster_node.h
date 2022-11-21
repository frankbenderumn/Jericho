#ifndef JERICHO_DIST_CLUSTER_NODE_H_
#define JERICHO_DIST_CLUSTER_NODE_H_

#include <string>
#include <deque>
#include <vector>
#include <unordered_map>
#include <dirent.h>

#include "server/defs.h"
#include "server/thread_pool.h"
#include "util/iters.h"
#include "message/message_broker.h"
#include "message/message_buffer.h"

#include "cluster/cluster_edge.h"
#include "cluster/cluster_index.h"

static int CLUSTER_ID = -1;
static int TICKET_ID = -1;

class Router;

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

class ClusterNode {

    std::string _host = "undefined";

    std::string _port = "undefined";

	std::string _dir = "undefined";

    bool _status = false;

	ClusterNodeType _type = CLUSTER_NODE_NULL;

	ClusterEdge* _edge = nullptr;

	ClusterIndex* _index;

	std::vector<std::string> _files;

	std::unordered_map<Client*, std::deque<MessageBroker*>> _brokers;

	int _id = -1;

	long long _timestamp = 0;

  public:

    ClusterNode(std::string host, std::string port, std::string dir, ClusterIndex* index);

	~ClusterNode();

	void brokerBroadcast(Router* router, Client* client, std::deque<MessageBuffer*> mq, MessageCallback callback);

	void brokerSend(Router* router, Client* client, std::string path, MessageBuffer* buf, std::string type = "", std::string content = "");

	void send2(Router* router, Client* client, std::string path, std::string type = "", std::string content = "");

	MessageBroker* poll(Client* client);

	void broadcastNaive(Router* router, Client* client, std::vector<std::pair<std::string, std::string>> pairs, std::string path, MessageCallback callback, std::string type = "", std::string content = "");

	void broadcast(Router* router, Client* client, std::string path, MessageCallback callback, std::string type = "", std::string content = "");

	void pulse(Router* router, Client* client, std::string path, MessageBroker* broker);
	
    void pingOne(Router* router, Client* client, ClusterNode* dest);

	void pingAll(Router* router, Client* client, std::vector<std::pair<std::string, std::string>> set = {});

	const int id() const;

	void send(Router* router, Client* client, std::string path, MessageBuffer* buf);

	MessageBuffer* buffer(Client* client, std::string path);

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

	void federate(Router* router, Client* client, std::string path, int epochs, int clients);

	void print();

};

#endif