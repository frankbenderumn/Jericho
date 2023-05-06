#ifndef JERICHO_SERIALIZER_SERIALIZER_H_
#define JERICHO_SERIALIZER_SERIALIZER_H_

#include "util/tools.h"
#include "server/response.h"
#include "picojson/picojson.h"

std::string http_status_to_json(std::string& http);

#endif