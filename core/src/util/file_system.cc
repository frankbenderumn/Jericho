#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include "openssl/md5.h"

#include "util/file_system.hpp"
#include "prizm/prizm.h"


void Jericho::FileSystem::write(const char* path, std::string toWrite, bool overwrite) {
    std::ofstream myfile;
    if (overwrite) {
        myfile.open(path, std::ios_base::in | std::ofstream::trunc);
    } else {
        myfile.open(path, std::ios_base::app | std::ios_base::in);
    }
    myfile << toWrite << "\n";
    myfile.close();
}

void Jericho::FileSystem::write(const char* path, const char* _toWrite, bool overwrite) {
    std::ofstream myfile;
    std::string toWrite(_toWrite);
    if (overwrite) {
        myfile.open(path, std::ios_base::in | std::ofstream::trunc);
    } else {
        myfile.open(path, std::ios_base::app | std::ios_base::in);
    }
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
        BCYA("JFS::readBinary: FILE SIZE IS: %i\n", (int)size);
    } else {
        BRED("JFS::readBinary: READ BINARY FILE ERROR ON PATH: %s\n", path);
    }

    if (size > 0) {
        std::vector<unsigned char> vec;
        char buffer[(int)size];
        std::ifstream t(path, std::ios::in | std::ios::binary);
        if (!t) {
            BRED("JFS::readBinary: ERROR READING BINARY FILE\n");
        }
        while (!(t.eof() || t.fail())) {
            t.read(buffer, size);
            vec.insert(vec.end(), buffer, buffer + t.gcount());
        }
        t.close();

        // std::string s = std::string(buffer);
        BCYA("JFS::readBinary: BUFFER SIZE IS: %i\n", (int)sizeof(buffer));
        BCYA("JFS::readBinary: BUFFER LEN IS: %i\n", (int)strlen(buffer));
        BCYA("JFS::readBinary: VEC SIZE: %i\n", (int)vec.size());
        std::string s(vec.begin(), vec.end());
        // std::cout << s;
        BCYA("JFS::readBinary: STRINGIFIED SIZE IS: %i\n", (int)s.size());
        return s;
    } else {
        BRED("JFS::readBinary: FILE SIZE IS 0 or ERROR\n");
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

void Jericho::FileSystem::writeBinary(const char* path, std::string content, bool overwrite) {
    BBLU("JFS::writeBinary: WRITING BINARY FILE...\n");
    BMAG("JFS::writeBinary: CONTENT SIZE %li\n", content.size());
    std::vector<char> v(content.begin(), content.end());
    BMAG("JFS::writeBinary: SIZE OF CHAR ARR: %li\n", v.size());
    std::ofstream t(path, std::ios::out | std::ios::binary);
    if (!t) {
        BRED("JFS::writeBinary: SOMETHING WENT WRONG WITH WRITING BINARY FILE\n");
    }
    t.write(v.data(), v.size() * sizeof(char));
    BMAG("JFS::writeBinary: BINARY FILE WROTE\n");
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

void mkDir(std::string dir) {

}

std::vector<std::string> publicDir() {
    return {};
}

bool Jericho::FileSystem::sanitize(std::string s, std::regex r) {
    return true;
}

int Jericho::FileSystem::parseJson(picojson::value& data, const char* json) {
    std::string err = picojson::parse(data, json);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        BRED("Failed to parse json file: %s\n", json);
        return -1;
    }

    if (!data.is<picojson::object>()) {
        BRED("Json file %s not an object!\n", json);
        return -1;
    }

    return 0;
}

int Jericho::FileSystem::readJson(picojson::value& data, const char* path) {
    std::ifstream nodes(path);
    std::stringstream buf;
    buf << nodes.rdbuf();
    std::string json = buf.str();
    // BCYA("Json is: %s\n", json.c_str());
    std::string err = picojson::parse(data, json);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        BRED("Failed to parse json file: %s\n", path);
        return -1;
    }

    if (!data.is<picojson::object>()) {
        BRED("Json file %s not an object!\n", path);
        return -1;
    }

    return 0;
}

int Jericho::FileSystem::exists(std::string pathname) {
    struct stat info;
    if( stat( pathname.c_str(), &info ) != 0 ) {
        printf( "cannot access %s\n", pathname.c_str() );
        return 0;
    } else if( info.st_mode & S_IFDIR ) {// S_ISDIR() doesn't exist on my windows 
        printf( "%s is a directory\n", pathname.c_str() );
        return 1;
    } else {
        printf( "%s is no directory\n", pathname.c_str() );
        return 2;
    }
}

bool Jericho::FileSystem::fileExists(std::string pathname) {
    return (2 == JFS::exists(pathname));
}

bool Jericho::FileSystem::dirExists(std::string pathname) {
    return (1 == JFS::exists(pathname));
}

void Jericho::FileSystem::mkDir(std::string dir) {
    if (JFS::exists(dir) == 0) {
        mkdir(dir.c_str(), 0777);
    } else {
        BRED("Failed to create directory: %s\n", dir.c_str());
    }
}

long Jericho::FileSystem::modifiedAt(const char* path) {
    BCYA("Path for modified at is: %s\n", path);
    struct stat attr;
    stat(path, &attr);
    struct timespec ts;
    timespec_get(&ts, attr.st_mtime);
    long t = ts.tv_sec;
    long l = attr.st_mtime;
    printf("Nano seconds: %ld\n", l);
    printf("Current time: %ld\n", time(0));
    printf("Last modified time: %s", ctime(&attr.st_mtime));
    return l;
}

size_t Jericho::FileSystem::size(const char* path) {
    FILE* fp;
    fp = fopen(path, "rb");
    size_t size;

    if (!fp) {
        BRED("JFS::size: Failed to open file '%s'\n", path);
        return -1;
    }

    // Get the current position of the file pointer
    long int current_pos = ftell(fp);

    // Seek to the end of the file
    fseek(fp, 0L, SEEK_END);

    // Get the current position, which is the size of the file
    size = ftell(fp);

    // Seek back to the original position
    fseek(fp, current_pos, SEEK_SET);

    return size;
}

std::string JFS::checksum(const std::string& filename, size_t chunk_size) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);

    std::array<unsigned char, 4096> buffer;
    while (ifs) {
        ifs.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        const auto bytes_read = ifs.gcount();
        if (bytes_read > 0) {
            MD5_Update(&md5_ctx, buffer.data(), bytes_read);
        }
    }

    std::array<unsigned char, MD5_DIGEST_LENGTH> md5_hash;
    MD5_Final(md5_hash.data(), &md5_ctx);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (const auto byte : md5_hash) {
        oss << std::setw(2) << static_cast<int>(byte);
    }

    return oss.str();
}