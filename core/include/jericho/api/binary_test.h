#ifndef JERICHO_API_BINARY_TEST_H_
#define JERICHO_API_BINARY_TEST_H_

#include "api/api_helper.h"
#include "api/api.h"

API(BinaryTest, {})
    BYEL("Executing binary test...\n");
    return "undefined";
}

#endif