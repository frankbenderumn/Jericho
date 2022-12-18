#include "message/message_buffer.h"
#include "message/message_broker.h"

void MessageBuffer::publish() {
    if (this->broker != NULL) {
        this->broker->publishMessage(this);
    } else {
        BRED("NULL BROKER BEING USED\n");
    }
}

void MessageBuffer::mark() {
    if (this->broker != NULL) {
        this->broker->markMessage(this);
    } else {
        BRED("NULL BROKER BEING USED\n");
    }
}

void MessageBuffer::dump() {
    BCYA("Message Buffer\n");
    BCYA("=====================\n");
    BCYA("Host       : %s\n", this->hostname.c_str());
    BCYA("Port       : %s\n", this->port.c_str());
    BCYA("Dir        : %s\n", this->dir.c_str());
    BCYA("fromPort   : %s\n", this->fromPort.c_str());
    BCYA("Path       : %s\n", this->path.c_str());
    BCYA("Sent       : %s\n", this->sent.c_str());
    BCYA("Type       : %s\n", this->type.c_str());
    BCYA("Ticket     : %i\n", this->ticket);
    BCYA("Sent       : %.50s\n", this->sent.c_str());
    BCYA("=====================\n");
}
