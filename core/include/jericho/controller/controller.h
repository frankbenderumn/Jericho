#ifndef JERICHO_CONTROLLER_CONTROLLER_H_
#define JERICHO_CONTROLLER_CONTROLLER_H_

#include <unordered_map>
#include <string>

#include "util/trace.h"
#include "system/route_functions.h"
#include "server/response.h"
#include "server/request.h"


enum EntType {
    ENT_NULL,
    ENT_USER
};

class Controller {
  protected:
    EntType type = ENT_NULL;
  public:
    Controller() {
        PCREATE;
    }

    ~Controller() {
        PDESTROY;
    }

    void create() {

    }

    void read() {

    }

    void update() {

    }

    void destroy() {

    }
};

#endif