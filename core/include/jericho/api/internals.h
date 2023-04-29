#ifndef JERICHO_API_INTERNALS_H_
#define JERICHO_API_INTERNALS_H_

#include "api/api_helper.h"
#include "api/api.h"

std::string generate_handshake(Request* req, int status) {
    return req->protocol + " " + std::to_string(status) + " " + HttpStatus::status_to_name(status);
}

API(Handshake, {})
    BMAG("API::Internals::Handshake:\n");
    MAG("\t%-16s: %s\n", "Protocol", req->protocol.c_str());
    MAG("\t%-16s: %s\n", "Path", req->path.c_str());
    MAG("\t%-16s: %s\n", "Protocol", req->method.c_str());
    std::string hs = generate_handshake(req, 100);
    BMAG("\tHandshake return: %s\n", hs.c_str());
    return hs;
}

#endif