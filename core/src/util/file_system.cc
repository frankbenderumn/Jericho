#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "util/file_system.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include "prizm/prizm.h"

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

std::string Jericho::FileSystem::readBinary(const char* path) {
    struct stat results;
    size_t size = -1;
    if (stat(path, &results) == 0 ) {
        size = results.st_size;
        BMAG("FILE SIZE IS: %i\n", (int)size);
    } else {
        BRED("READ BINARY FILE ERROR ON PATH: %s\n", path);
    }

    if (size > 0) {
        std::vector<unsigned char> vec;
        char buffer[(int)size];
        std::ifstream t(path, std::ios::in | std::ios::binary);
        if (!t) {
            BRED("ERROR READING BINARY FILE\n");
        }
        while (!(t.eof() || t.fail())) {
            t.read(buffer, size);
            vec.insert(vec.end(), buffer, buffer + t.gcount());
        }
        t.close();

        // std::string s = std::string(buffer);
        BCYA("BUFFER SIZE IS: %i\n", (int)sizeof(buffer));
        BCYA("BUFFER LEN IS: %i\n", (int)strlen(buffer));
        BCYA("VEC SIZE: %i\n", (int)vec.size());
        std::string s(vec.begin(), vec.end());
        // std::cout << s;
        BCYA("STRINGIFIED SIZE IS: %i\n", (int)s.size());
        return s;
    } else {
        BRED("FILE SIZE IS 0 or ERROR\n");
    }
    return "";
}

void Jericho::FileSystem::readCBinary(const char* path, unsigned char* buffer) {

    // for chunked version
    // while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    // {
    //     // process bytesRead worth of data in buffer
    // }

    struct stat results;
    size_t size = -1;
    if (stat(path, &results) == 0 ) {
        size = results.st_size;
        BMAG("FILE SIZE IS: %i\n", (int)size);
    } else {
        BRED("READ BINARY FILE ERROR ON PATH: %s\n", path);
    }

    FILE* fp = fopen(path,"rb");
    if (!fp) { BRED("FAILED TO OPEN FILE"); }

    char* contents = (char*)calloc(size, sizeof(unsigned char));

    BMAG("CONTENT BUFFER SIZE: %li\n", sizeof(contents));

    fread(contents, size, 1, fp);

    fclose(fp);  

    // memcpy(buffer, contents, size);

    BMAG("BINARY FILE SIZE: %li\n", sizeof(contents));
    // BMAG("BINARY FILE LEN: %li\n", strlen(buffer));

    free(contents);
}

void Jericho::FileSystem::writeBinary(const char* path, std::string content) {
    BBLU("WRITING BINARY FILE...\n");
    BMAG("CONTENT SIZE %li\n", content.size());
    std::vector<char> v(content.begin(), content.end());
    BMAG("SIZE OF CHAR ARR: %li\n", v.size());
    std::ofstream t(path, std::ios::out | std::ios::binary);
    if (!t) {
        BRED("SOMETHING WENT WRONG WITH WRITING BINARY FILE\n");
    }
    t.write(v.data(), v.size() * sizeof(char));
    BMAG("BINARY FILE WROTE\n");
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
    return {};
}

bool Jericho::FileSystem::sanitize(std::string s, std::regex r) {
    return true;
}


