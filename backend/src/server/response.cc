#include "server/response.h"
#include <sys/wait.h>

const char* http_response[] = {
    // Accepted
    "200 OK",
    "201 Created",
    "202 Accepted",
    "203 Non-Authorization Information",
    "204 No Content",
    "205 Reset Content",
    "206 Partial Content",
    // Redirection
    "300 Multiple Choices",
    "301 Moved Permanently",
    "302 Found",
    "303 See Other",
    "304 Not Modified",
    "305 Use Proxy",
    "306 (Unused)",
    "307 Temporary Direct",
    // Client Error
    "400 Bad Request",
    "401 Unauthorized",
    "402 Payment Required",
    "403 Forbidden",
    "404 Not Found",
    "405 Method Not Allowed",
    "406 Not Acceptable",
    "407 Proxy Authentication Required",
    "408 Request Timeout",
    "409 Conflict",
    "410 Gone",
    "411 Length Required",
    "412 Precondition Failed",
    "413 Request Entity Too Large",
    "414 Request-Uri Too Long",
    "415 Unsupported Media Type",
    "416 Requested Range Not Satisfiable",
    "417 Expectation Failed",
    // Server Error
    "500 Internal Server Error",
    "501 Not Implemented",
    "502 Bad Gateway",
    "503 Service Unavailable",
    "504 Gateway Timeout",
    "505 HTTP Version Not Unsupported"
};

const char* http_header[92] = {
"Accept",
"Accept-CH",
// "ExperimentalNon-StandardDeprecated"
// "Accept-CH-Lifetime"
"Accept-Charset",
"Accept-Encoding",
"Accept-Language",
"Accept-Patch",
"Accept-Post",
"Accept-Ranges",
"Access-Control-Allow-Credentials",
"Access-Control-Allow-Headers",
"Access-Control-Allow-Methods",
"Access-Control-Allow-Origin",
"Access-Control-Expose-Headers",
"Access-Control-Max-Age",
"Access-Control-Request-Headers",
"Access-Control-Request-Method",
"Age",
"Allow",
"Alt-Svc",
"Authorization",
"Cache-Control",
"Clear-Site-Data",
"Connection",
"Content-Disposition",
// "ExperimentalNon-StandardDeprecated"
// "Content-DPR"
"Content-Encoding",
"Content-Language",
"Content-Length",
"Content-Location",
"Content-Range",
"Content-Security-Policy",
"Content-Security-Policy-Report-Only",
"Content-Type",
"Cookie",
"Cross-Origin-Embedder-Policy",
"Cross-Origin-Opener-Policy",
"Cross-Origin-Resource-Policy",
"Date",
// "Experimental"
// "Device-Memory"
"Digest",
"DNT",
// "Experimental"
// "Downlink"
// "ExperimentalNon-StandardDeprecated"
// "DPR"
// "Experimental"
// "Early-Data"
// "Experimental"
// "ECT"
"ETag",
"Expect",
"Expect-CT",
"Expires",
// "Experimental"
// "Feature-Policy"
"Forwarded",
"From",
"Host",
"If-Match",
"If-Modified-Since",
"If-None-Match",
"If-Range",
"If-Unmodified-Since",
"Keep-Alive",
// "Non-StandardDeprecated"
// "Large-Allocation"
"Last-Modified",
"Link",
"Location",
"NEL",
"Origin",
// "Deprecated"
// "Pragma"
"Proxy-Authenticate",
"Proxy-Authorization",
// "Deprecated"
// "Public-Key-Pins"
// "Deprecated"
// "Public-Key-Pins-Report-Only"
"Range",
"Referer",
"Referrer-Policy",
"Retry-After",
// "Experimental"
// "RTT"
"Save-Data",

/** TODO: research of interest */
// "Experimental"
// "Sec-CH-UA"
// "Experimental"
// "Sec-CH-UA-Arch"
// "Experimental"
// "Sec-CH-UA-Bitness"
// "Experimental"
// "Sec-CH-UA-Full-Version"
// "Experimental"
// "Sec-CH-UA-Full-Version-List"
// "Experimental"
// "Sec-CH-UA-Mobile"
// "Experimental"
// "Sec-CH-UA-Model"
// "Experimental"
// "Sec-CH-UA-Platform"
// "Experimental"
// "Sec-CH-UA-Platform-Version"

"Sec-Fetch-Dest",
"Sec-Fetch-Mode",
"Sec-Fetch-Site",
"Sec-Fetch-User",
"Sec-WebSocket-Accept",
"Server",
"Server-Timing",
"Service-Worker-Navigation-Preload",
"Set-Cookie",
"SourceMap",
"Strict-Transport-Security",
"TE",
"Timing-Allow-Origin",
"Tk",
"Trailer",
"Transfer-Encoding",
"Upgrade",
"Upgrade-Insecure-Requests",
"User-Agent",
"Vary",
"Via",
// "ExperimentalNon-StandardDeprecated"
// "Viewport-Width"
"Want-Digest",
// "Deprecated"
// "Warning"
// "ExperimentalDeprecated"
// "Width"
"WWW-Authenticate",
"X-Content-Type-Options",
"X-DNS-Prefetch-Control",
// "Non-Standard"
// "X-Forwarded-For"
// "Non-Standard"
// "X-Forwarded-Host"
// "Non-Standard"
// "X-Forwarded-Proto"
"X-Frame-Options",
"X-XSS-Protection"
};

void file_read(const char* path, char* buffer);

void send_400(Client* client) {
    const char* c400 = "HTTP/1.1 400 Bad Request\r\n"
                        "Connection: close\r\n"
                        "Content-length: 11\r\n\r\nBad Request";
    SSL_write(client->ssl, c400, strlen(c400));
    // drop_client(client);
}

/** WARN: Max of 4096 may be too small */
//probably something unsfe going on here
void send_404(Client* client) {
    const char* c404 = "HTTP/1.1 404 Not Found\r\n"
                    "Connection: close\r\n";
    DEBUG("Size of c404 is: %i\n", (int)strlen(c404));
    char* result = (char*)malloc(strlen(c404) + 4096);
    char buffer[4096];
    DEBUG("404 size of buffer: %i\n", (int)strlen(buffer));
    strncpy(result, c404, strlen(c404));
    DEBUG("Pre result is: %s\n", result);
    file_read("./scaffold/response/404.html", buffer);
    // printf("Buffer: %s\n", buffer);
    char buffer2[4096];
    int l = strlen(buffer);
    int n = l;
    int count = 0;
    while(n!=0) {  
       n=n/10;  
       count++;  
    }  
    snprintf(buffer2, count + 30 + 1, "Content-length: %i\r\n\r\n", l);
    strncat(result, buffer2, count + 30);
    strncat(result, buffer, strlen(buffer) + strlen(result));
    // printf("Result: %s\n", result);

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
    DEBUG("Buffer size is: %i\n", (int)sizeof(buffer));
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
    DEBUG("resource: %s\n", resource);
    char* vals = strstr(resource, "\r\n\r\n");
    if (vals == NULL) {
        DEBUG("SUBSTRING NOT FOUND!\n");
        exit(1);
    }
    DEBUG("vals: %s\n", vals + 4);
    int position = vals - resource;
    DEBUG("idx: %d\n", position);
    char* valcopy = vals;
    char* token;
    PLOGV(LSERVER, "CGI", 3, "Resource: %s", resource);

    int number, statval;
    int child_pid;
    DEBUG("%d: I'm the parent !\n", getpid());
    child_pid = fork();
    if(child_pid == -1) { 
        DEBUG("could not fork! \n");
        exit( 1 );
    } else if(child_pid == 0) {
        execl("./cgi/login.sh", valcopy);
    } else {
        DEBUG("PID %d: waiting for child\n", getpid());
        waitpid( child_pid, &statval, WUNTRACED
                    #ifdef WCONTINUED       /* Not all implementations support this */
                            | WCONTINUED
                    #endif
                    );
        if(WIFEXITED(statval)) {
            DEBUG("Child's exit code %d\n", WEXITSTATUS(statval));
        } else {
            DEBUG("Child did not terminate with exit\n");
        }
    }
}

void serve_resource(Client* conn, const char* path) {
    char addr_buffer[16];
    client_get_address(conn, addr_buffer);
    PLOGV(LSERVER, "RESOURCE", 6, "Serving resource: %s", path);
    
    // if at root file
    if (strcmp(path, "/") == 0) path = "/index.html";

    // need more sanity checks

    // security precaution for long url, ex: https://localhost:8080////////////////////index.html
    // and buffer overflow
    // if (strlen(path) > 100) { 
    //     send_400(conn->socket);
    //     return;
    // }

    // security precaution to prevent unpriveleged access https://localhost:8080/../../keys/key.pem
    // if (strstr(path, "..")) {
    //     send_404(conn->socket);
    //     return;
    // }

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
        PLOGV(LSERVER, "RESOURCE", 1, "Can not serve file does not exist");
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

void contains_header() {

}

void extract_headers() {

}

void extract_header() {
    // strtok_r
}

