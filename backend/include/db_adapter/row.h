#ifndef ROW_H_
#define ROW_H_

#include "db_adapter/attribute.h"
#include <pqxx/pqxx>

class Row {
  public:
    Row(const std::vector<pqxx::field>& row) {
      _row = row;
      // for (auto const &field: row) {
      //     std::cout << field.c_str() << '\t';
      // }
    }

    std::vector<pqxx::field> GetRow() const { return _row; }

    void Dump() const {
      for (auto f : _row) {
        std::cout << f.c_str() << "\t";
      }
    }

    std::vector<std::string> Stringify() const {
      std::vector<std::string> result = {};
      for (auto f : _row) {
        result.push_back(f.c_str());
      }
      return result;
    }

  private:
    std::unordered_map<std::string, Attribute*> _attributes;
    std::vector<pqxx::field> _row;
};

#endif