#ifndef JERICHO_MESSAGE_MESSAGE_H_
#define JERICHO_MESSAGE_MESSAGE_H_

#include <string>

#include "message/message_broker.h"

static int RES_TICKET_ID = -1;

enum MessageFormatType {
    MSGF_NULL,
    MSGF_HTTP,
    MSGF_JSON,
    MSGF_JSON_LINES,
    MSGF_XML,
    MSGF_YAML,
    MSGF_IRIS,
    MSGF_HSTS,
    MSGF_SEC,
    MSGF_BLOCKCHAIN
};

enum MessageType {
    MSG_NULL,
    MSG_RESPONSE,
    MSG_REQUEST
};

class Message {
  protected:
    int status = -1;
    int _ticket = -1;
    MessageType _type;
    MessageBroker* _broker = nullptr;
    bool fulfilled = false;
  public:
    Message() {}
    virtual ~Message() {}
    const MessageType type() const {
        return _type;
    }
};

class MessageRequest : public Message {
    std::string request = "undefined";

  public:
    MessageRequest() {}
    ~MessageRequest() {}
};

class MessageResponse : public Message {
    std::string response = "undefined";

  public:
    MessageResponse() {}
    ~MessageResponse() {}
};

#endif