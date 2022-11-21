#ifndef CLUSTER_CLUSTER_QUORUM_H_
#define CLUSTER_CLUSTER_QUOROM_H_

#include <string>
#include "prizm/prizm.h"

enum FLStatusType {
    FL_NULL,
    FL_JOINED,
    FL_TRAINING,
    FL_DELIVERED,
    FL_DORMANT,
    FL_DROPPED,
    FL_WAITLISTED,
    FL_BLACKLISTED,
    FL_EXITED
};

struct ClusterQuorum {
    std::string host;
    std::string port;
    FLStatusType status; 
    long long timestamp;
    long long updated;
    int rounds;
    int served;

    /** should be QuorumMember for better readability */
    ClusterQuorum(std::string host, std::string port, long long timestamp) {
        this->host = host;
        this->port = port;
        this->timestamp = timestamp; 
        this->updated = timestamp;
        this->status = FL_JOINED;
        this->served = 0;
    }

    ~ClusterQuorum() { BYEL("DELETING QUORUM\n"); }

    void update() {
        this->updated = std::time(NULL);
    }

    std::string serializeStatus() {
        std::string result;
        switch(status) {
            case FL_JOINED:
                result = "joined";
                break;
            case FL_DROPPED:
                result = "dropped";
                break;
            case FL_DORMANT:
                result = "dormant";
                break;
            case FL_WAITLISTED:
                result = "waitlisted";
                break;
            case FL_BLACKLISTED:
                result = "blacklisted";
                break;
            case FL_TRAINING:
                result = "training";
                break;
            default:
                result = "undefined";
                break;
        }
        return result;
    }

    std::string serialize() {
        return "{\"host\": "+host+", \"port\": "+port+", \"served\": "+std::to_string(served)+", \"timestamp\": "+std::to_string(timestamp)+", \"status\": "+serializeStatus()+"}";
    }
};

#endif