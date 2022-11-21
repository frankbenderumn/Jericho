#include "api/api_helper.h"

std::string pipe(std::string command) {
    std::string result;
    FILE* fp;
    int status = -1;

    fp = popen(command.c_str(), "r");
    if (fp == NULL) { return "PIPE ERROR"; }

    int someLen = 0;
    int lineLen = 10000;
    char *line;
    line = (char *)malloc(sizeof(char) * lineLen);
    if (line == NULL) {
        return "PIPE ERROR";
    }
    while (fgets(line, lineLen, fp)) {
        BMAG("LINE: %s\n", line);
        result += std::string(line);
        someLen += lineLen;
        if (someLen != lineLen) {
            line = (char *)realloc(line, sizeof(char) * someLen);
        }
        if (line == NULL) {
            return "PIPE ERROR";
        }
    }

    printf("Size of line is: %i\n", (int)result.size());
    BGRE("LINE: %s\n", result.c_str());
    free(line);

    status = pclose(fp);
    if (status == -1) {
        return "PIPE ERROR";
    }

    return result;
}

bool apiValidate(std::string substr, std::regex rgx) {
    RegexIter it(substr.begin(), substr.end(), rgx);
    RegexIter end;
    if (std::distance(it, end) != 0) {
        return true;
    }
    return false;
}