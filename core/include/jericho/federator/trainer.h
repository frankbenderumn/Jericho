#ifndef JERICHO_FEDERATOR_TRAINER_H_
#define JERICHO_FEDERATOR_TRAINER_H_

#include "federator/fed_node.h"

class Trainer : public FedNode {
    std::string _partition = "undefined";
    std::string _train = "train.py";
    std::string _weights;

  public:
    Trainer(std::string dataset) {
      PCREATE;
      _dataset = dataset;
      _role = FED_ROLE_CLIENT;
    }

    void config(const std::string& host, const std::string& port) {
      this->_dir = "./public/cluster/" + port;
      this->_host = host;
      this->_port = port;
      BYEL("Trainer::config:\b");
      YEL("\tdir : %s\n", this->_dir.c_str());
      YEL("\thost: %s\n", this->_host.c_str());
      YEL("\tport: %s\n", this->_port.c_str());
    }

    ~Trainer() { PDESTROY; }

    std::string train(std::string model) {
      BGRE("Model path is: %s\n", model.c_str());
      std::string command_path = "python3 ./py/torch_load.py " + this->_dir;
      std::string results = pipe(command_path);
      std::string wts = this->_dir + "/mnist_train.wt";
      BBLU("Results: %s\n", results.c_str());
      BGRE("Weights path is: %s\n", wts.c_str());
      _weights = wts;
      return results;
    }

    const std::string& weights() const { return _weights; }
};

#endif