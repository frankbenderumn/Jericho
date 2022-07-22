#ifndef ATTRIBUTE_H_
#define ATTRIBUTE_H_

#include <pqxx/pqxx>
#include <string>
#include <iostream>

class Attribute {
  public:
    Attribute(std::string field) {
        _field = field;
    }

    void Print() {
        std::cout << _field.c_str() << std::endl;
    }

  private:
    std::string _field;

};

#endif