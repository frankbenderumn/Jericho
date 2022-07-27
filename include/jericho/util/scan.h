#ifndef UTIL_SCAN_H_
#define UTIL_SCAN_H_

#include <stdio.h>
#include <stdbool.h>
#include <regex.h>

#include "server/defs.h"

bool scan(const char* regex_val, char* request);

#endif