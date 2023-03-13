#ifndef JERICHO_CONTROLLER_AUTH_H_
#define JERICHO_CONTROLLER_AUTH_H_

#include "controller/user.h"
#include "session/session.h"

namespace oauth {
    Response* login(System* system, Request* req, jericho::Session* sesh);
}

#endif