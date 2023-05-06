#ifndef MESSAGE_MESSAGE_JOB_H_
#define MESSAGE_MESSAGE_JOB_H_

class MessageJob {
    int ticket = -1;
    int status;
	std::string id;
	std::string type = "none";
	std::string content;
	std::string checksum;

  public:
    MessageJob(std::string id, std::string type, std::string checksum) {
        this->id = id;
		this->type = type;
		this->checksum = checksum;
    }

	void append(std::string chunk) {

	}
};

#endif