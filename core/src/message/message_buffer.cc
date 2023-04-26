#include "message/message_buffer.h"
#include "message/message_broker.h"

void MessageBuffer::publish() {
    if (!this->broker.expired()) {
        std::shared_ptr<MessageBroker> locked = this->broker.lock();
        if (locked) {
            locked->publishMessage(this);
        }
    } else {
        BRED("NULL BROKER BEING USED\n");
    }
}

void MessageBuffer::mark() {
    if (!this->broker.expired()) {
        std::shared_ptr<MessageBroker> locked = this->broker.lock();
        if (locked) {
            locked->markMessage(this);
        }
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
    BCYA("fromPort   : %s\n", this->fromHost.c_str());
    BCYA("fromPort   : %s\n", this->fromPort.c_str());
    BCYA("toPort     : %s\n", this->toHost.c_str());
    BCYA("toPort     : %s\n", this->toPort.c_str());
    BCYA("Path       : %s\n", this->path.c_str());
    BCYA("Sent       : %.100s\n", this->sent.c_str());
    BCYA("Type       : %s\n", this->type.c_str());
    BCYA("Ticket     : %i\n", this->ticket);
    BCYA("Protocol   : %s\n", this->protocol.c_str());
    BCYA("URL        : %s\n", this->url.c_str());
    for (auto head : this->headers) {
    BCYA("%-11s: %s\n", head.first.c_str(), head.second.c_str());
    }
    BCYA("=====================\n");
}

MessageBuffer::MessageBuffer(int& ticket, std::string _url) {
    PCREATE;
    URL* url = new URL(_url);
    if (url->ip) {
        this->port = url->port;

        this->toHost = url->host;
        this->toPort = url->port;

        this->protocol = url->protocol;

        this->path = url->path;
        this->type = url->type;

        if (url->protocol == "jfl") {
            this->dir = "/cluster";
        } else {
            this->dir = "undefined";
        }

        this->timestamp = std::chrono::high_resolution_clock::now();

        // if (broker != nullptr) {
        //     buf->broker = broker;
        // }

        this->id = ++ticket;
        this->url = url->url;
    } else {
        BRED("MessageBuffer: url is not of type ip\n");
    }
    delete url;
}