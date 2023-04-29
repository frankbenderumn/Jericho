#include "prizm2/prizm.h"
#include "message/message.h"

class MessageUnit : public ITest {
    MTRACE(MessageUnit);
  protected:
    void precondition() {
        msg = new Message(++ticket, "https://127.0.0.1:8081", "127.0.0.1", "8085");
    }
    void postcondition() {
        delete msg;
    }
    Message* msg;
    int ticket = 0;
};

PTEST(MessageUnit, serialize1) {
    msg->headers["Content-Type"] = "application/json";
    std::string s = msg->serialize();
    BBLU("%s\n", s.c_str());
    printf("\n");
    // PASSERT(1, 1);
}


PTEST(MessageUnit, chunk_serialize) {
    msg->chunk("hello", 0, 2048, 5096);
    msg->headers["Content-Type"] = "application/json";
    std::string s = msg->serialize();
    BBLU("%s\n", s.c_str());
    printf("\n");
    // PASSERT(1, 1);
}

PTEST(MessageUnit, http_status1) {
    msg->status(100);
    std::string s = msg->serialize();
    BBLU("%s\n", s.c_str());
}