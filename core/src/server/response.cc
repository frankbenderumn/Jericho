#include "server/response.h"
#include "server/resource.h"
#include <sys/wait.h>
#include "iris/iris.h"
#include "server/defs.h"
#include "server/socket.h"
#include "server/client.h"

using namespace Jericho;

void file_read(const char* path, char* buffer);

void serve(Client* client, const char* content) {
    SSL_write(client->ssl, content, strlen(content));
}

void send_400(Client* client) {
    const char* c400 = "HTTP/1.1 400 Bad Request\r\n"
                        "Connection: close\r\n"
                        "Content-length: 11\r\n\r\nBad Request";
    SSL_write(client->ssl, c400, strlen(c400));
    // drop_client(client);
}

//probably something unsfe going on here
void send_404(Client* client) {
    const char* c404 = "HTTP/1.1 404 Not Found\r\n"
                    "Connection: close\r\n"
                    "Content-length: 200\r\n\r\n";
    printf("Size of c404 is: %i\n", (int)strlen(c404));
    char* result = (char*)malloc(strlen(c404) + 4096);
    char buffer[4096];
    printf("404 size of buffer: %i\n", (int)strlen(buffer));
    strncpy(result, c404, strlen(c404));
    BMAG("Result is: %s\n", result);
    file_read("frontend/response/404.html", buffer);
    strncat(result, buffer, strlen(buffer) + strlen(result));
    BBLU("Result is: %s\n", result);
    SSL_write(client->ssl, result, strlen(result));
    free(result);
    // drop_client(client);
}

size_t file_size(FILE* fp) {
    fseek(fp, 0L, SEEK_END); // move to end of file
    size_t size = ftell(fp); // get size of file in bytes
    rewind(fp); 
    return size;
}

void file_read(const char* path, char* buffer) {
    FILE* fp = fopen(path, "rb"); // open file, set fds to read in bytes

    if (!fp) { PFAIL(ESERVER, "File does not exist!\n"); }

    size_t sz = file_size(fp);
    printf("Buffer size is: %i\n", (int)sizeof(buffer));
    if (sz > 4096) { PFAIL(ESERVER, "File is too large (4096 bytes is the limit)"); }

    char file[4096];
    int r = fread(file, 1, 4096, fp);
    strncpy(buffer, file, sizeof(file));
}

void build_header(char* dest, HttpHeaderType hheader, const char* value) {
    const char* header = http_header[hheader];
    strncpy(dest, header, sizeof(header));
    if (sizeof(value) > 31) {
        PFAIL(ESERVER, "Http header value too long, can not be greater the 31 chars");
    }
    strncpy(dest, value, sizeof(value));
    dest += '\0';
}

std::string header(Client* client, HttpResponseType hresponse) {
    std::string http_version = "HTTP/1.1 ";
    std::string http_method = http_version + std::string(http_response[hresponse]) + "\r\n";
    int i = atoi(http_response[hresponse]);
    std::string http_connection = "Connection: keep-alive\r\n";
    if (i == 5 || i == 4 || i == 3) {
        http_connection = "Connection: close\r\n";
    }
    std::string http_content_length = std::string("Content-length: ") + "100" + "\r\n";
    // int length = (int)strlen(header) - 4;
    std::string http_response = http_method + http_connection + http_content_length;
    SSL_write(client->ssl, http_response.c_str(), sizeof(http_response.c_str()));
}


void send_response(Client* client, HttpResponseType hresponse) {
    const char* first = "HTTP/1.1 ";
    const char* header = http_response[hresponse];
    int i = atoi(&header[0]);
    char response[2048];
    strncpy(response, first, sizeof(first));
    strncat(response, header, sizeof(header));
    strncat(response, "\r\n", 3);
    if (i == 5 || i == 4 || i == 3) {
        strncat(response, "Connection: close\r\n", sizeof("Connection: close\r\n"));
    } else {
        strncat(response, "Connection: keep-alive\r\n", sizeof("Connection: keep-alive\r\n"));
    }
    int length = (int)strlen(header) - 4;
    strncat(response, "Content-length: ", sizeof("Content-length: "));
    char val = length+'0';
    strncat(response, &val, 1);
    strncat(response, "\r\n", 3);
    response[2048] = 0;
    // add str size limit on debug
    DEBUG("SENDING: %s\n", response);
    SSL_write(client->ssl, response, sizeof(response));
}

void post_resource(Client* conn, char* resource) {
    GRE("resource: %s\n", resource);
    char* vals = strstr(resource, "\r\n\r\n");
    if (vals == NULL) {
        BRED("SUBSTRING NOT FOUND!\n");
        // exit(1);
    }
    printf("vals: %s\n", vals + 4);
    int position = vals - resource;
    printf("idx: %d\n", position);
    char* valcopy = vals;
    char* token;
    
    int number, statval;
    int child_pid;
    printf("%d: I'm the parent !\n", getpid());
    child_pid = fork();
    if(child_pid == -1) { 
        printf("could not fork! \n");
        exit( 1 );
    } else if(child_pid == 0) {
        execl("./cgi/login.sh", valcopy);
    } else {
        printf("PID %d: waiting for child\n", getpid());
        waitpid( child_pid, &statval, WUNTRACED
                    #ifdef WCONTINUED       /* Not all implementations support this */
                            | WCONTINUED
                    #endif
                    );
        if(WIFEXITED(statval))
            printf("Child's exit code %d\n", WEXITSTATUS(statval));
        else
            printf("Child did not terminate with exit\n");
    }
}

void socket_write(SSL* ssl, const void *buf, unsigned size) {
    int writedTotal = 0;
    const char *ccBuf = reinterpret_cast<const char *>(buf);

    while (writedTotal < size) {
        int writedPart = SSL_write(ssl, ccBuf + writedTotal, size - writedTotal);
        if (writedPart < 0) {
            std::string error = "SSL_write error = ";
            error += (std::to_string(writedPart) + ".");
            throw std::runtime_error(error);
        }
        writedTotal += writedPart;
    }
}

void resource::serve_raw2(Client* conn, const char* content, size_t content_size) {
    SSL_write(conn->ssl, content, content_size); // send bytes
    printf("DONE WRITING DIST\n");
}

void resource::serve_http(Client* conn, Client** clients, const char* content, std::string type) {
    if (strlen(content) > 65000) {
        BRED("CONTENT TO SEND IS TO LARGE! NEED TO FRAGMENT BUFFER!\n");
    }
    char buffer[65536];
    sprintf(buffer, "HTTP/1.1 200 OK\r\n");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Connection: close\r\n");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Content-Length: %lu\r\n", strlen(content));
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Content-Type: %s\r\n", type.c_str());
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Access-Control-Allow-Origin: %s\r\n", "*");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Access-Control-Allow-Methods: %s\r\n", "GET, POST, OPTIONS");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Access-Control-Allow-Headers: %s\r\n", "Content-Type");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Access-Control-Max-Age: %s\r\n", "86400");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Jericho: %s\r\n", "true");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "\r\n");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    BBLU("bug found? response::serve_http: %li\n", strlen(content));
    SSL_write(conn->ssl, content, strlen(content)); // send bytes
}

void resource::serve_http2(Client* conn, Client** clients, const char* content, size_t content_size, std::string type) {
    if (content_size > 4096) {
        BRED("CONTENT TO SEND IS TOO LARGE! NEED TO FRAGMENT BUFFER!\n");
    }
    char buffer[4096];
    int len = 0;
    // len += snprintf(buffer + len, sizeof(buffer) - len, "HTTP/1.1 200 OK\r\n");
    // len += snprintf(buffer + len, sizeof(buffer) - len, "Connection: close\r\n");
    // len += snprintf(buffer + len, sizeof(buffer) - len, "Content-Length: %lu\r\n", content_size);
    // len += snprintf(buffer + len, sizeof(buffer) - len, "Content-Type: %s\r\n", type.c_str());
    // len += snprintf(buffer + len, sizeof(buffer) - len, "Access-Control-Allow-Origin: %s\r\n", "*");
    // len += snprintf(buffer + len, sizeof(buffer) - len, "Access-Control-Allow-Methods: %s\r\n", "GET, POST, OPTIONS");
    // len += snprintf(buffer + len, sizeof(buffer) - len, "Access-Control-Allow-Headers: %s\r\n", "Content-Type");
    // len += snprintf(buffer + len, sizeof(buffer) - len, "Access-Control-Max-Age: %s\r\n", "86400");
    // len += snprintf(buffer + len, sizeof(buffer) - len, "Jericho: %s\r\n", "true");
    // len += snprintf(buffer + len, sizeof(buffer) - len, "\r\n");
    // SSL_write(conn->ssl, buffer, len);
    // std::string temp = "hello larry";
    // if (content_size == 201) {
    //     len += snprintf(buffer + len, sizeof(buffer) - len, "hello larry");
    // } else {
    //     len += snprintf(buffer + len, sizeof(buffer) - len, "%s", content);
    // }
    int written = SSL_write(conn->ssl, content, content_size);
    BBLU("Sent bytes is: %i\n", written);
    std::string sent(content, content_size);
    // BBLU("resource::serve_http2: Sending data\n");
    // for (auto& c : sent) {
    //     if (c == '\0') {
    //         printf("\033[1;34m0\033[0m");
    //     } else {
    //         printf("%c", c);
    //     }
    // }
    // printf("\n");
}

void resource::serve_cxx(System* sys, Client* conn, Client** clients, const char* path) {
    Router* router = sys->router();
    char addr_buffer[16];
    // printf("Path to serve is: %s\n", path);
    client_get_address(conn, addr_buffer);
    DEBUGC(4, "serve_resource %s %s\n", addr_buffer, path);
    std::string p = std::string(path);
    std::string dir = PUBLIC_DIRECTORY;
    if (p == "") p = "/index.html";
    if (p == "/") p = "/index.html";

    std::string full_path = dir + p;
    // BWHI("FULL PATH: %s\n", full_path.c_str());
    FILE* fp = fopen(full_path.c_str(), "rb"); // open file, set fds to read in bytes

    if (!fp) { 
        resource::error(router, conn, "404"); 
        return; 
    }

    if (strstr(path, "..")) {
        resource::error(router, conn, "404");
        return;
    }

    if (strlen(path) > 100) { 
        resource::error(router, conn, "404");
        return;
    }

    size_t sz = file_size(fp);
    const char* content = get_content_type(full_path.c_str());

    std::string file_contents = JFS::read(full_path.c_str());
    if (file_contents.find("$[idoc]$") != std::string::npos) {
        BYEL("Interpretting...\n");
        file_contents = iris::interpret(router, full_path.c_str());
    }
    // BBLU("DONE INTERPRETING\n");
    std::string ext = std::string(content);
    sz = file_contents.size();
    // BBLU("CONTNEN: %s\n", ext.c_str());
    // BMAG("SZ IS: %i\n", (int)strlen(file_contents.c_str()));
    // BWHI("Directory is: %s\n", dir.c_str());
    // BWHI("Sending: %s\n", file_contents.c_str());

    char buffer[8192];
    sprintf(buffer, "HTTP/1.1 200 OK\r\n");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Connection: keep-alive\r\n");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Content-Length: %lu\r\n", sz);
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Content-Type: %s\r\n", content);
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Access-Control-Allow-Origin: %s\r\n", "https://localhost");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "\r\n");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    
    if (ext == "text/html") {
        SSL_write(conn->ssl, file_contents.c_str(), strlen(file_contents.c_str())); // send bytes
    } else {
        // read file contents into multiple 1024 packets
        int r = fread(buffer, 1, 1024, fp);
        while (r) {
            SSL_write(conn->ssl, buffer, r); // send bytes
            r = fread(buffer, 1, 1024, fp); // read another 1024
        }
    }

    fclose(fp); // close file
}

void resource::serve_raw(Client* conn, Client** clients, const char* message) {
    SSL_write(conn->ssl, message, strlen(message)); // send bytes
    printf("DONE WRITING DIST\n");
}

void serve_resource(Client* conn, const char* path) {
    char addr_buffer[16];
    client_get_address(conn, addr_buffer);
    BLU("serve_resource %s %s\n", addr_buffer, path);

    // if at root file
    if (strcmp(path, "/") == 0) path = "/index.html";

    // need more sanity checks
    //may need more sanitization (xss attack prevention with regex/octet substition)
    // csrf tokens for same origin verification
    // session authentication for login 
    // timeouts for inactivity

    char full_path[128];
    sprintf(full_path, "frontend%s", path);

#ifdef _WIN32 // dumb windows and their FAT file system
    char* p = full_path;
    while (*p) {
        if (*p == '/') *p = '\\';
        ++p;
    }
#endif

    FILE* fp = fopen(full_path, "rb"); // open file, set fds to read in bytes

    if (!fp) {
        send_404(conn); // file does not exist
        return;
    }

    size_t sz = file_size(fp);

    // get content-type i.e. text/html, application/json
    const char* content = get_content_type(full_path); 

    // #define BSIZE 1024
    char buffer[4096];

    // if size of file is exceptionally large, send over multiple sends?
    // do sz check

    sprintf(buffer, "HTTP/1.1 200 OK\r\n");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Connection: close\r\n");
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Content-Length: %lu\r\n", sz);
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "Content-Type: %s\r\n", content);
    SSL_write(conn->ssl, buffer, strlen(buffer));
    sprintf(buffer, "\r\n");
    SSL_write(conn->ssl, buffer, strlen(buffer));

    // read file contents into multiple 1024 packets
    int r = fread(buffer, 1, 4096, fp);
    while (r) {
        SSL_write(conn->ssl, buffer, r); // send bytes
        r = fread(buffer, 1, 4096, fp); // read another 1024
    }

    fclose(fp); // close file
    // drop_client(conn, &clients); // close conn connection
    // close thread
}

Response::Response(System* sys, Request* req) {
    PCREATE;
    this->sys = sys;
    this->client = req->client;
    this->request = req;
    this->headers = req->headers;
}


std::string Response::generate() {
    char addr_buffer[16];
    // printf("Path to serve is: %s\n", path);
    client_get_address(request->client, addr_buffer);
    DEBUGC(4, "serve_resource %s %s\n", addr_buffer, req->path.c_str());
    std::string p = request->path;
    std::string dir = PUBLIC_DIRECTORY;

    if (p == "") p = "/index.html";
    if (p == "/") p = "/index.html";

    std::string full_path = dir + p;
    // BWHI("FULL PATH: %s\n", full_path.c_str());
    FILE* fp = fopen(full_path.c_str(), "rb"); // open file, set fds to read in bytes

    if (!fp) { 
        resource::error(sys->router(), request->client, "404"); 
        return "ERROR";
    }

    if (strstr(request->path.c_str(), "..")) {
        resource::error(sys->router(), request->client, "404");
        return "ERROR";
    }

    if (strlen(request->path.c_str()) > 100) { 
        resource::error(sys->router(), request->client, "404");
        return "ERROR";
    }

    size_t sz = file_size(fp);
    const char* content = get_content_type(full_path.c_str());

    std::string file_contents = JFS::read(full_path.c_str());
    if (file_contents.find("$[idoc]$") != std::string::npos) {
        BYEL("Interpretting...\n");
        file_contents = iris::interpret(sys->router(), full_path.c_str());
    }
    // BBLU("DONE INTERPRETING\n");
    std::string ext = std::string(content);
    sz = file_contents.size();
    // BBLU("CONTNEN: %s\n", ext.c_str());
    // BMAG("SZ IS: %i\n", (int)strlen(file_contents.c_str()));
    // BWHI("Directory is: %s\n", dir.c_str());
    // BWHI("Sending: %s\n", file_contents.c_str());
    std::string buffer;

    buffer += "HTTP/1.1 200 OK\r\n";
    buffer += "Connection: keep-alive\r\n";
    buffer += "Content-Length: " + std::to_string(sz) + "\r\n";
    buffer += "Content-Type: " + std::string(content) + "\r\n";
    // buffer += "Access-Control-Allow-Origin: https://localhost\r\n"; 
    for (auto cookie : cookies) {
        std::string cook = cookie->generate();
        cook.pop_back();
        buffer += "Set-Cookie: " + cook + "\r\n";
    }
    buffer += "\r\n";
    buffer += file_contents;
    // char buffer[8192];
    // sprintf(buffer, "HTTP/1.1 200 OK\r\n");
    // SSL_write(conn->ssl, buffer, strlen(buffer));
    // sprintf(buffer, "Connection: keep-alive\r\n");
    // SSL_write(conn->ssl, buffer, strlen(buffer));
    // sprintf(buffer, "Content-Length: %lu\r\n", sz);
    // SSL_write(conn->ssl, buffer, strlen(buffer));
    // sprintf(buffer, "Content-Type: %s\r\n", content);
    // SSL_write(conn->ssl, buffer, strlen(buffer));
    // sprintf(buffer, "Access-Control-Allow-Origin: %s\r\n", "https://localhost");
    // SSL_write(conn->ssl, buffer, strlen(buffer));
    // sprintf(buffer, "\r\n");
    // SSL_write(conn->ssl, buffer, strlen(buffer));
    char buf[buffer.size()];
    strncpy(buf, buffer.c_str(), buffer.size());

    // BBLU("Response is: \n%s\n", buf);
    
    // if (ext == "text/html") {
    //     SSL_write(request->client->ssl, file_contents.c_str(), strlen(file_contents.c_str())); // send bytes
    // } else {
    //     // read file contents into multiple 1024 packets
    //     int r = fread(buf, 1, 1024, fp);
    //     while (r) {
    //         SSL_write(request->client->ssl, buffer.c_str(), r); // send bytes
    //         r = fread(buf, 1, 1024, fp); // read another 1024
    //     }
    // }

    fclose(fp); // close file

    std::string s(buf);
    return s;
}

void Response::serve() {
    std::string res = this->generate();
    BBLU("Response\n=========================================\n%s\n", res.c_str());
    // for (unsigned i = 0; i < res.length(); i += 1024) {
    //     std::string chunk = res.substr(i, 1024);
    //     BYEL("Chunk:\n%s\n", chunk.c_str());
        SSL_write(this->request->client->ssl, res.c_str(), res.size());
    // }
}

void contains_header() {

}

void extract_headers() {

}

void extract_header() {
    // strtok_r
}

