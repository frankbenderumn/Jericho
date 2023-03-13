#ifndef JERICHO_CONTROLLER_USER_CONTROLLER_H_
#define JERICHO_CONTROLLER_USER_CONTROLLER_H_

#include "controller/controller.h"

class UserController : public Controller {
  public:
    UserController() { PCREATE; this->type = ENT_USER; }
    ~UserController() { PDESTROY; }
    bool login(std::unordered_map<std::string, std::string> args) {
        if (!prizm::contains_key(args, std::string("Username"))) return false;
        if (!prizm::contains_key(args, std::string("Password"))) return false;
        if (args["Password"] == "pass1234" && 
            args["Username"] == "joey") return true;
        return false;
    }
};

#endif