#ifndef FEDERATE_FEDERATOR_H_ 
#define FEDERATE_FEDERATOR_H_   
    
#include <ctime>

#include "prizm/prizm.h"

class Federator {
    bool _needsAgg = false;
    int _numClients = 0;
    long long _aggStart = 0;
    long long _aggWait = 10 * 1000;
    int _numRounds = 0;
    int _roundCt = 0;
    int _clientCt = 0;
    bool _active = false;
    std::string _bytes = "undefined";
    int _id;

  public:
    Federator(int numClients, int numRounds, int secWait, int id) {
        _numClients = numClients;
        _numRounds = numRounds;
        _aggWait = secWait;
        _id = id;
    }

    ~Federator() {
        BYEL("DESTORYING FEDERATOR\n");
    }

    const long long startTime() const {
        return _aggStart;
    }

    const long long waitTime() const {
        return _aggWait;
    }

    std::string bytes() const { return _bytes; }

    void bytes(std::string bytes) { _bytes = bytes; } 

    const int rounds() const { return _numRounds; }

    const int clients() const { return _numClients; }

    void start() {
        _active = true;
        _aggStart = std::time(NULL);
    }

    void stop() {
        _active = false;
    }

    const int id() const { return _id; }

    const int round() const { return _roundCt; }

    const int clientCt() const { return _clientCt; }

    void incClientCt() { _clientCt++; }

    void clientCt(int ct) { _clientCt = ct; } 

    const bool active() const { return _active; }

    void finishRound() {
        _roundCt++;
        _aggStart = std::time(NULL);
        _clientCt = 0;
        if (_roundCt == _numRounds) {
            BGRE("FEDERATION COMPLETED\n");
            _active = false;
        }
    }
};

#endif