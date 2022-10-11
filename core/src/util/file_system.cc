#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "util/file_system.hpp"

void Jericho::FileSystem::write(const char* path, std::string toWrite) {
    std::ofstream myfile;
    myfile.open(path, std::ios_base::app | std::ios_base::in);
    myfile << toWrite << "\n";
    myfile.close();
}

std::string Jericho::FileSystem::read(const char* path) {
    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

std::string Jericho::FileSystem::getDir(std::string name) {
    std::string fileBuffer = "";
    // if (sanitize(name, std::regex("([A-Za-z0-9_-]/?)+[A-Za-z0-9_-]+\\.([A-Za-z]+|[A-Za-z]+\\.[A-Za-z]+){1}"))) {
    if (sanitize(name, std::regex("*"))) {
        FILE* stream;
        const int max_buffer = 256;
        char buffer[max_buffer];
        std::string cmd = "ls " + name;
        cmd.append(" 2>&1");

        stream = popen(cmd.c_str(), "r");

        if (stream) {
            while (!feof(stream))
                if (fgets(buffer, max_buffer, stream) != NULL) fileBuffer.append(buffer);
            pclose(stream);
        }
    } else {
        // PFAIL(EINVARG, "Invalid directory access\n");
    }

    return fileBuffer;
}

std::vector<std::string> publicDir() {
    
}

bool Jericho::FileSystem::sanitize(std::string s, std::regex r) {
    return true;
}


