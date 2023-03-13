#ifndef JERICHO_URL_URL_H_
#define JERICHO_URL_URL_H_

#include <string>
#include <unordered_map>
#include "prizm/prizm.h"
#include "util/trace.h"

enum ProtocolType {
    PROTOCOL_NULL,
    PROTOCOL_HTTP,
    PROTOCOL_JERICHO,
    PROTOCOL_SMTP,
    PROTOCOL_RPC
};

enum ContentType {
    CONTENT_NULL,
    CONTENT_TEXT,
    CONTENT_JSON,
    CONTENT_BINARY,
    CONTENT_HTML,
    CONTENT_XML,
    CONTENT_CSV,
    CONTENT_YAML,
    CONTENT_OCTET,
    CONTENT_IRIS,
    CONTENT_JSON_LINES,
    CONTENT_BLOB,
    CONTENT_BASE64
};

class Url {
    std::unordered_map<std::string, std::string> _headers;
    ContentType _contentType = CONTENT_NULL;
    ProtocolType _protocolType = PROTOCOL_NULL;
  public:
    Url() { PCREATE; }
    ~Url() { PDESTROY; }
    void headers(std::string bytes) {}
};

namespace url {

Url* parse(std::string bytes) {

}

}

#endif