#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "util/file_system.hpp"
#include <dirent.h>

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

std::vector<std::string> Jericho::FileSystem::getDir(std::string dir) {
    std::vector<std::string> fileBuffer;
    DIR* dr;
    struct dirent* en;
    dr = opendir(dir.c_str()); //open all or present directory

    if ( !dr ) {
        printf("\033[1;31mINVALID DIRECTORY PROVIDED FOR CLUSTER NODE: %s\033[0m\n", dir.c_str());
        // exit(1);
    }

    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            printf("%s\n", en->d_name); //print all directory name
            std::string path = std::string(en->d_name);
            if (path != "." && path != "..") {
                fileBuffer.push_back(path);
            }
        }
        closedir(dr); //close all directory
    }

    return fileBuffer;
}

std::vector<std::string> publicDir() {
    
}

bool Jericho::FileSystem::sanitize(std::string s, std::regex r) {
    return true;
}


