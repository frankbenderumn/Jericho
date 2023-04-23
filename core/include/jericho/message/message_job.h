#ifndef MESSAGE_MESSAGE_JOB_H_
#define MESSAGE_MESSAGE_JOB_H_

class MessageJob {
  int ticket = -1;
  int status;
  public:
    MessageJob(int ticket) {
        this->ticket = ticket;
    }
};

#endif