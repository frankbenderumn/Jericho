#include "message/message.h"
#include "message/message_broker.h"
#include "server/http_status.h"

void Message::publish() {
    if (!this->broker.expired()) {
        std::shared_ptr<MessageBroker> locked = this->broker.lock();
        if (locked) {
            locked->publishMessage(this);
        }
    // } else {
    //     BRED("NULL BROKER BEING USED\n");
    }
}

void Message::mark() {
    if (!this->broker.expired()) {
        std::shared_ptr<MessageBroker> locked = this->broker.lock();
        if (locked) {
            locked->markMessage(this);
        }
    // } else {
    //     BRED("NULL BROKER BEING USED\n");
    }
}

void Message::dump() {
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

Message::Message(int& ticket, std::string _url, std::string fromHost, std::string fromPort) {
    PCREATE;
    URL* url = new URL(_url);
    if (url->ip) {
        this->port = url->port;

        this->toHost = url->host;
        this->toPort = url->port;

        this->hostname = fromHost;
        this->fromHost = fromHost;
        this->fromPort = fromPort;
        this->port = fromPort;

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
        BRED("Message: url is not of type ip\n");
    }
    delete url;
}

void Message::status(int statusCode) {
    this->method = "STATUS";
    this->statusCode = statusCode;
    this->statusName = HttpStatus::status_to_name(statusCode);
}


std::string Message::serialize() {
    int result = 1;
    char* buffer = (char*)malloc((4096));

    std::string message = this->sent;
    BMAG("Message::serialize: this->sent.size(): %li\n", this->sent.size());
    std::unordered_map<std::string, std::string> headers = this->headers;
    std::string type = this->type;
    std::string protocol = this->protocol;
    if (protocol == "https") protocol = "HTTP/1.1";
    if (protocol == "job") protocol = "JOB";
    std::string hostname = this->fromHost;
    std::string port = this->fromPort;
    std::string path = this->path;

    if (path.size() > 200) {
        BRED("Message::serialize: Path size > 200\n");
        return "";
    }

    if (hostname.size() > 50) {
        BRED("Message::serialize: Host size > 50\n");
        return "";
    }

    if (port.size() > 6) {
        BRED("Message::serialize: Port size > 6\n");
        return "";
    }

    if (method != "STATUS") {
        sprintf(buffer, "%s ", method.c_str());
        sprintf(buffer + strlen(buffer), "%s ", path.c_str());
        sprintf(buffer + strlen(buffer), "%s\r\n", protocol.c_str());
    } else {
        sprintf(buffer, "%s %i %s\r\n", protocol.c_str(), this->statusCode, this->statusName.c_str());
    }

    sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname.c_str(), port.c_str());
    if (strlen(buffer) > 4000) { BRED("Fetch::send_request: Buffer too large\n"); return 0; }
    sprintf(buffer + strlen(buffer), "Connection: keep-alive\r\n");
    if (strlen(buffer) > 4000) { BRED("Fetch::send_request: Buffer too large\n"); return 0; }
    sprintf(buffer + strlen(buffer), "User-Agent: Jericho 0.1 alpha\r\n");
    if (strlen(buffer) > 4000) { BRED("Fetch::send_request: Buffer too large\n"); return 0; }
    // sprintf(buffer + strlen(buffer), "Access-Control-Allow-Origin: %s\r\n", "*");

    if (type == "bin") {
        sprintf(buffer + strlen(buffer), "Content-Length: %li\r\n", message.size());
        sprintf(buffer + strlen(buffer), "Content-Type: %s\r\n", "application/octet-stream");        
    } else {
        sprintf(buffer + strlen(buffer), "Content-Length: %li\r\n", message.size());
    }

    int chunkTotal = this->fileSize / this->chunkSize;
// range vs. content-range get vs. post
    if (chunkTotal > 1 && type == "bin") {
        sprintf(buffer + strlen(buffer), "Transfer-Encoding: chunked %s\r\n", "base64");
        if (method == "GET") {
            sprintf(buffer + strlen(buffer), "Range: bytes=%li/%li\r\n", message.size(), message.size());
        } else if (method == "POST") {
            if ((chunkNum + 1) * chunkSize > fileSize) {
                sprintf(buffer + strlen(buffer), "Content-Range: bytes %li-%li/%li\r\n", chunkNum*chunkSize, (chunkNum+1)*chunkSize, fileSize);
            } else { 
                sprintf(buffer + strlen(buffer), "Content-Range: bytes %li-%li/%li\r\n", chunkNum*chunkSize, fileSize, fileSize);
            }
        }
    } else if (chunkTotal > 1) {
        sprintf(buffer + strlen(buffer), "Transfer-Encoding: %s\r\n", "base64");
        if (method == "GET") {
            sprintf(buffer + strlen(buffer), "Range: bytes=%li/%li\r\n", message.size(), message.size());
        } else if (method == "POST") {
            if ((chunkNum + 1) * chunkSize > fileSize) {
                sprintf(buffer + strlen(buffer), "Content-Range: bytes %li-%li/%li\r\n", chunkNum*chunkSize, (chunkNum+1)*chunkSize, fileSize);
            } else { 
                sprintf(buffer + strlen(buffer), "Content-Range: bytes %li-%li/%li\r\n", chunkNum*chunkSize, fileSize, fileSize);
            }
        }
    } else if (type == "bin") {
        sprintf(buffer + strlen(buffer), "Content-Encoding: %s\r\n", "base64");
    }

    for (auto head : headers) {
        sprintf(buffer + strlen(buffer), "%s: %s\r\n", head.first.c_str(), head.second.c_str());
    }
    sprintf(buffer + strlen(buffer), "\r\n");

    if (type == "bin") {
        std::string encoded = jcrypt::base64::encode_url(message);
        sprintf(buffer + strlen(buffer), "%s", encoded.data());
    } else {
        sprintf(buffer + strlen(buffer), "%s", message.data());
    }
    return std::string(buffer);
}

std::string Message::serialize_directive() {
    std::string result;
    result += directive + "\r\n";
    for (auto h : headers) {
        result += h.first + ": " + h.second + "\r\n";
    }
    result += "\r\n" + sent;
    return result;
}