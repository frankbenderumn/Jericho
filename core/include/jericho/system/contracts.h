#ifndef JERICHO_SYSTEM_CONTRACTS_H_
#define JERICHO_SYSTEM_CONTRACTS_H_

#include "prizm/prizm.h"

// contract guidelines
// typedef int (*ContractFunction)(std::unordered_map<std::string, std::string>);

int pre(std::unordered_map<std::string, std::string> args) {
    if (!prizm::contains_key(args, std::string("Host"))) { return -1; }
    if (!prizm::contains_key(args, std::string("Content"))) { return -1; }
    return 0;
}


// currently redundant but may have future use case
int post(std::unordered_map<std::string, std::string> args) {
    if (!prizm::contains_key(args, std::string("Host"))) { return -1; }
    return 0;
}  

#endif