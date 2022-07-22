#include "db_adapter/database.h"
#include "openssl/sha.h"
#include "prizm/file_system.h"

int lenHelper(unsigned x) {
    if (x >= 1000000000) return 10;
    if (x >= 100000000)  return 9;
    if (x >= 10000000)   return 8;
    if (x >= 1000000)    return 7;
    if (x >= 100000)     return 6;
    if (x >= 10000)      return 5;
    if (x >= 1000)       return 4;
    if (x >= 100)        return 3;
    if (x >= 10)         return 2;
    return 1;
}

int printLen(int x) {
    return x < 0 ? lenHelper(-x) + 1 : lenHelper(x);
}

int main(int argc, char* argv[]) {
    const char* ins = argv[1];
    printf("PATH : %s\n", getenv("ENV_VAR"));
    printf("TOKEN: %s\n", ins);
    write_file("login.log", ins);
    std::string str(ins);

    std::cout << "STRING: " << str << std::endl;

    Database* db = new Database;

    RECORD one = db->QueryNames("users");
    printf("one size: %i\n", (int)one.size());
    printf("1: %s\n", one.at(0).c_str());
    printf("2: %s\n", one.at(1).c_str());
    printf("3: %s\n", one.at(2).c_str());
    printf("4: %s\n", one.at(3).c_str());

    // std::string str = "username=johnsnow&password=pass1234&hello=something&new=whoa";

    size_t pos = 0;
    int k = 0;
    RECORD record;
    while ((pos = str.find("&")) != std::string::npos) {
        std::string tk = str.substr(0, pos);
        str.erase(0, pos + strlen("&"));
        record.push_back(tk);
    }
    record.push_back(str);

    for (auto r : record) {
        printf("r: %s\n", r.c_str());
    }

    std::vector<std::pair<std::string, std::string>> attrs;
    for (int i = 0; i < record.size(); i++) {
        std::string s = record.at(i);
        size_t pos = 0;
        int k = 0;
        std::pair<std::string, std::string> attr;
        while ((pos = s.find("=")) != std::string::npos) {
            std::string tk = s.substr(0, pos);
            std::string tk2 = s.substr(pos + 1, s.size());
            attr.first = tk;
            attr.second = tk2;
            s.erase(0, pos + strlen("="));
            attrs.push_back(attr);
        }
    }

    std::string username;
    std::string password;

    for (auto a : attrs) {
        printf("first: %s\n", a.first.c_str());
        printf("second: %s\n", a.second.c_str());
        if (a.first == "username") {
            username = a.second;
        } else if (a.first == "password") {
            password = a.second;
        }
    }

    printf("username: %s\n", username.c_str());
    printf("password: %s\n", password.c_str());

    const unsigned char* toHash = (unsigned char*)password.c_str();
    char* hash = NULL;

    // toHash = calloc(SHA256_DIGEST_LENGTH, sizeof(const unsigned char));
    size_t plen = strlen((const char*) toHash);

    long int t = (int)time(NULL);
    // long int t = 1653854050; // test case
    printf("Timestamp: %ld\n",t);
    printf("sizeof time: %li\n",sizeof(t));
    char tbuffer[printLen(t) + 1];
    snprintf(tbuffer, printLen(t) + 1, "%s", one.at(3).c_str());
    printf("tbuffer: %s\n", tbuffer);
    printf("sizeof tbuffer: %i\n", (int)sizeof(tbuffer));
    size_t tlen = strlen((const char*) tbuffer);

    char pdigest[tlen + plen + 1];
    printf("pdigest: %ld\n", sizeof(pdigest));
    strncpy(pdigest, tbuffer, tlen + 1);
    printf("pdigest: %s\n", pdigest);
    strncat(pdigest, (char*)toHash, plen + 1);
    printf("pdigest: %s\n", pdigest);
    pdigest[tlen + plen + 1] = 0;
    printf("pdigest len: %i\n", (int)strlen(pdigest));
    
    hash = (char*)calloc(SHA256_DIGEST_LENGTH, sizeof(char));
    size_t len2 = strlen((const char*) pdigest);
    SHA256((unsigned char*)pdigest, len2, (unsigned char*) hash);

    int term = strlen(hash) * 2;
    printf("term: %i\n", term);
    char buffer[65];

    // buffer overflow risk? 
    printf("strlen: %i\n", (int)strlen(buffer));
    int length = 0;
    for(unsigned int j = 0; j < SHA256_DIGEST_LENGTH; j ++) {
	    length += sprintf(buffer + length, "%02hhX", hash[j]);
        printf("%02hhX", hash[j]);
    }
    buffer[65] = 0;
    printf("\n");
    printf("buffer: %s\n", buffer);
    printf("sizeof buffer: %i\n", (int)sizeof(buffer));
    free(hash);

    std::string hashed(buffer);
    std::string salt(tbuffer);

    // std::string query = "INSERT INTO users VALUES(1, '" + username + "','" + hashed + "','" + salt + "')";
    printf("hashed: %s\n", hashed.c_str());
    printf("record: %s\n", one.at(2).c_str());

    if (hashed == one.at(2)) {
        printf("\033[1;32mSUCCESS\n\033[0m");
        exit(1);
    } else {
        printf("\033[1;31mFAILURE\n\033[0m");
        exit(-1);
    }
    // for sign up
    // std::vector<std::string> v = {username, hashed, salt};
    // db->Insert("users", v);

    return 0;
}

