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
    }

    ~Trainer() { PDESTROY; }

    void train(std::string model) {
      _weights = "dfddfsdfsdfsdf\0dsfsd\0zdfdsfsdfsdfsdf\0\0\0\0THIS IS A BINARY TEST";
    }

    const std::string& weights() const { return _weights; }
};

#endif