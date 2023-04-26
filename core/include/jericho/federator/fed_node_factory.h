#ifndef JERICHO_FEDERATOR_FED_NODE_FACTORY_H_
#define JERICHO_FEDERATOR_FED_NODE_FACTORY_H_

#include <string>
#include <vector>

#include "util/file_system.hpp"
#include "util/trace.h"
#include "federator/trainer.h"
#include "federator/aggregator.h"

class FedNodeFactory {
  public:
    FedNodeFactory() { PCREATE; }
    ~FedNodeFactory() { PDESTROY; }
    FedNode* create(std::string url, std::string role, std::string configPath, std::vector<std::string> parents, std::vector<std::string> children) {
        picojson::value data;
        if (JFS::readJson(data, configPath.c_str()) < 0) {
            BRED("FedNodeFactory::create: Failed to read json file '%s'\n", configPath.c_str());
            return nullptr;
        }

        if (!data.is<picojson::object>()) {
            BRED("FedNodeFactory::create: FedNode config file '%s' is not a JSON object\n", configPath.c_str());
            return nullptr;
        }

        picojson::object config = data.get<picojson::object>();

        // client config
        // {
        //     "train": "py/torch_load.py",
        //     "partition": "1",
        //     "data": "MNIST"
        // }

        // aggregator config
        // {
        //     "train": "py/torch_load.py",
        //     "partition": "1",
        //     "rounds": 2,
        //     "data": "MNIST",
        //     "quorum": 1,
        //     "lr": 0.0001,
        //     "optimizer": "adagrad"
        // }

        // central config
        // {
        //     "train": "py/torch_load.py",
        //     "partition": "1",
        //     "rounds": 10,
        //     "data": "MNIST",
        //     "quorum": 1,
        //     "lr": 0.001,
        //     "optimizer": "adam"
        // }

        if (role == "agg") {
            Aggregator* agg = new Aggregator("MNIST");
            GRE("FedNodeFactory::create: Created Aggregator Node!\n");
            agg->parents(parents);
            agg->children(children);
            agg->dump();
            return agg;
        } else if (role == "cli") {
            // Trainer* client = new Trainer("MNIST");
            // GRE("FedNodeFactory::create: Created Trainer Node!\n");
            // client->parents(parents);
            // client->dump();
            // return client;
        } else if (role == "main") {
            Aggregator* agg = new Aggregator("MNIST");
            GRE("FedNodeFactory::create: Created Leader Node!\n");
            agg->leader = 1;
            agg->parents(parents);
            agg->children(children);
            agg->dump();
            return agg;            
        }
        return nullptr;
    }
};

#endif