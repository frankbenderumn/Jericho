#ifndef MESSAGE_REALM_H_
#ifndef MESSAGE_REALM_H_

#include <string>

#include "prizm/prizm.h"
#include "util/trace.h"

class Realm {
    std::string _path;
  public:
    Realm(std::string path) {
        PCREATE;
        _path = path
    }

    ~Realm() {
        PDESTROY;
    }

};

#endif