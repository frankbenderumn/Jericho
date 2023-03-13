#ifndef JERICHO_UTIL_TRACE_H_
#define JERICHO_UTIL_TRACE_H_

#include <string>
#include <utility>

#include "prizm/prizm.h"

namespace prizm {

std::pair<int, int> substrcmp(std::string s1, std::string s2);

}

struct TracePair {
    std::string className = "undefined";
    int count = 0;
    TracePair* prev = nullptr;
    TracePair* next = nullptr;
    TracePair(std::string _className, int _count) {
        this->className = _className;
        this->count = _count;
    }
    ~TracePair() {
        BYEL("Clearing trace pair %s\n", className.c_str());
    }
};
 
struct Trace {
    TracePair* head = nullptr;
    int numPairs = 0;
    ~Trace() {
        BYEL("DESTROYING TRACE...\n");
        this->dump();
        while (head != nullptr) {
            TracePair* temp = head->next;
            delete head;
            head = temp;
        }
    }
    void trace(std::string className) {
        // YEL("Tracing... %s\n", className.c_str());
        size_t trieIdx = 0;
        char bref = className.at(trieIdx);
        TracePair* prev = nullptr;
        TracePair* next = nullptr;

        if (head == nullptr) {
            // BRED("Head is null. Creating trace pair...\n");
            TracePair* pair = new TracePair(className, 1);
            head = pair;
            numPairs++;
            if (head == nullptr) {
                // BRED("HEAD is still null\n");
            }
            return;
        }

        TracePair* temp = head;
        // RED("To insert: %s\n", className.c_str());
        int marked = 0;
        bool subset = false;
        while (temp != nullptr) {
            std::pair<int, int> subpair = prizm::substrcmp(className, temp->className);
            TracePair* pair = nullptr;
            if (subpair.first == -1) {
                if (temp->prev == nullptr) {
                    // BBLU("Adding front (left)\n");
                    TracePair* pair = new TracePair(className, 1);
                    temp->prev = pair;
                    pair->next = temp;
                    temp = pair;
                    head = temp;
                    numPairs++;
                    this->dump();
                    break;
                } else if (marked == -1) {
                    // BBLU("Insert merge here (left)\n");
                    TracePair* pair = new TracePair(className, 1);
                    temp->prev->next = pair;
                    pair->prev = temp->prev;
                    pair->next = temp;
                    temp->prev = pair;
                    numPairs++;
                    marked = 0;
                    break;
                } else {
                    // BYEL("Marked for insertion (left)\n");
                    marked = 1;
                }
            } else if (subpair.first == 0) {
                // BGRE("Match detected\n");
                temp->count++;
                return;
            } else if (subpair.first == 1) {
                if (temp->next == nullptr) {
                    // BBLU("Adding last (right)\n");
                    TracePair* pair = new TracePair(className, 1);
                    temp->next = pair;
                    pair->prev = temp;
                    // temp = pair;
                    numPairs++;
                    // this->dump();
                    break;
                } else if (marked == 1) {
                    // BBLU("Insert merge here (right)\n");
                    TracePair* pair = new TracePair(className, 1);
                    temp->prev->next = pair;
                    pair->prev = temp->prev;
                    pair->next = temp;
                    temp->prev = pair;
                    numPairs++;
                    marked = 0;
                    break;
                } else {
                    // BYEL("Marked for insertion (right)\n");
                    marked = -1;
                }
            } else if (subpair.first == 2) {
                // GRE("Undersubset found\n");
                // BYEL("Current: %s, %s\n", className.c_str(), temp->className.c_str());
                break;
            } else if (subpair.first == 3) {
                // GRE("Oversubset found\n");
                // BYEL("Current: %s, %s\n", className.c_str(), temp->className.c_str());
                break;
            }

            temp = temp->next;
        }
        // this->dump();
    }

    void dump() {
        MAG("Prizm Trace Dump\n");
        // if (head != nullptr && head->className != "undefined") {
        //     CYA("Class: %s, count: %i\n", head->className.c_str(), head->count);
        // } else {
        //     BRED("Trace - No classes registered\n");
        // }
        if (head == nullptr) {
            BRED("Nothing to dump. Head is null!\n");
        }

        TracePair* temp = head;
        while (temp != nullptr && temp != NULL) {
            CYA("Class: %s, count: %i\n", temp->className.c_str(), temp->count);
            temp = temp->next;
        }
    }

    void clean(std::string name) {
        TracePair* temp = head;
        while (temp != nullptr) {
            if (temp->className == name) {
                temp->count--;
            }
            temp = temp->next;
        }
        // this->dump();
    }

    void scan() {
        TracePair* temp = head;
        bool allClear = true;
        while (temp != nullptr) {
            if (temp->count != 0) {
                BRED("Leak detected: %s, %i\n", temp->className.c_str(), temp->count);
                allClear = false;
            }
            temp = temp->next;
        }
        // this->dump();
        if (allClear) {
            BGRE("Prizm Trace: No leaks detected!\n");
        } else {
            BRED("Prizm Trace: Leak detected!\n");
        }
    }
};

std::string parseClassName(std::string bytes);

extern Trace* GLOBAL_TRACE;

#define TRACE

#ifdef TRACE
#define PCREATE \
    { std::string __s = __PRETTY_FUNCTION__; \
    std::string __className = parseClassName(__s); \
    GLOBAL_TRACE->trace(__className); \
    }
#define PDESTROY \
    { std::string __s = __PRETTY_FUNCTION__; \
    std::string __className = parseClassName(__s); \
    GLOBAL_TRACE->clean(__className); }
#define PCREATEC(x) { \
    std::string s(x); \
    GLOBAL_TRACE->trace(s); }
#define PDESTROYC(x) { \
    std::string s(x); \
    GLOBAL_TRACE->clean(s); }
#define TRACEDUMP \
    GLOBAL_TRACE->dump();
#define TRACESCAN GLOBAL_TRACE->scan();
#else
#define PCREATEC(x) ;
#define PDESTROYC(x) ;
#define PDESTROY
#define PCREATE
#define TRACEDUMP
#define TRACESCAN
#endif

#define TRACEDESTROY delete GLOBAL_TRACE;

void exit_func();

#define TRACEMEM const int mem_trace = std::atexit(exit_func);

class TraceTest {
  public:
    TraceTest() {
        PCREATE;
    }
    ~TraceTest() {
        PDESTROY;
    }
};

class ExampleTrace {
  public:
    ExampleTrace() {
        PCREATE;
    }
    ~ExampleTrace() {
        PDESTROY;
    }
};

class TraceTest2 {
  public:
    TraceTest2() {
        PCREATE;
    }
    ~TraceTest2() {
        PDESTROY;
    }
};

class SraceTest {
  public:
    SraceTest() {
        PCREATE;
    }
    ~SraceTest() {
        PDESTROY;
    }
};

class AmpleTrace {
  public:
    AmpleTrace() {
        PCREATE;
    }
    ~AmpleTrace() {
        PDESTROY;
    }
};

class BampleTrace {
  public:
    BampleTrace() {
        PCREATE;
    }
    ~BampleTrace() {
        PDESTROY;
    }
};

class BampleTraced {
  public:
    BampleTraced() {
        PCREATE;
    }
    ~BampleTraced() {
        PDESTROY;
    }
};

class BampleTracedd {
  public:
    BampleTracedd() {
        PCREATE;
    }
    ~BampleTracedd() {
        PDESTROY;
    }
};

class CampleTrace {
  public:
    CampleTrace() {
        PCREATE;
    }
    ~CampleTrace() {
        PDESTROY;
    }
};

class UTrace {
  public:
    UTrace() {
        PCREATE;
    }
    ~UTrace() {
        PDESTROY;
    }
};

class Srace {
  public:
    Srace() {
        PCREATE;
    }
    ~Srace() {
        PDESTROY;
    }
};

class TracerTest {
  public:
    TracerTest() {
        PCREATE;
    }
    ~TracerTest() {
        PDESTROY;
    }
};

class TraceTests {
  public:
    TraceTests() {
        PCREATE;
    }
    ~TraceTests() {
        PDESTROY;
    }
};

class AAmpleTrace {
  public:
    AAmpleTrace() {
        PCREATE;
    }
    ~AAmpleTrace() {
        PDESTROY;
    }
};

class AmppleTrace {
  public:
    AmppleTrace() {
        PCREATE;
    }
    ~AmppleTrace() {
        PDESTROY;
    }
};


#endif