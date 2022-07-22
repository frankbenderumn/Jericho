#include "server/smtp.h"

void get_input(const char* prompt, char* buffer) {
    BYEL("%s", prompt);
    buffer[0] = 0;
    fgets(buffer, MAXINPUT, stdin);
    const int read = strlen(buffer);
    printf("Read should be 1 :: %i\n", read);
    if (read > 0) buffer[read - 1] = 0;
}

void send_format(SOCKET server, const char* text, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, text);
    // use vsnprintf for bo prevention
    vsnprintf(buffer, 64, text, args);
    // upgrade to ssl?
    send(server, buffer, strlen(buffer), 0);
    printf("C: %s", buffer);
    va_end(args);
}

int parse_mail_response(const char* response) {
    const char* k = response;
    if (!k[0] || !k[1] || !k[2]) return 0;
    for (; k[3]; ++k) {
        if (k == response || k [-1] == '\n') {
            if (isdigit(k[0]) && isdigit(k[1]) && isdigit(k[2])) {
                if (k[3] != '-') {
                    if (strstr(k, "\r\n")) {
                        return strtol(k, 0, 10);
                    }
                }
            }
        }
    }
    return 0;
}

void wait_on_response(SOCKET server, int expecting) {
    char response[MAXRESPONSE];
    char* p = response;
    char* end = response + MAXRESPONSE;

    int code = 0;

    do {
        int bytes_received = recv(server, p, end - p, 0);
        if (bytes_received  < 1) {
            PFAIL(ESOCK, "Connection dropped.\n");
        }

        p += bytes_received;

        *p = 0;

        if (p == end) {
            PFAIL(ESZ, "Server response too large:\n%s\n", response);
        }

        code = parse_mail_response(response);
    } while (code == 0);

    if (code != expecting) {
        // truncation warning??
        PFAIL(ESERVER, "Error from server:\n%s\n", response);
    }

    printf("S: %s", response);
}

SOCKET connect_to_mail_host(const char* hostname, int port) {
    return socket_create(hostname, port, 1, AF_INET, SOCK_STREAM);
}

int mail_servicer(const char* hostname, const char* recipient, const char* sender) {
    if (sizeof(hostname)/sizeof(char) > 16) {
        PFAIL(ESZ, "Hostname can't be greater than 15 chars\n");
    }

    if (sizeof(hostname)/sizeof(char) > MAXINPUT) {
        PFAIL(ESZ, "Recipient can't be greater than %i chars\n", MAXINPUT);
    }

    if (sizeof(hostname)/sizeof(char) > MAXINPUT) {
        PFAIL(ESZ, "Sender can't be greater than %i chars\n", MAXINPUT);
    }

    YEL("Connecting to host: %s\n", hostname);

    // TODO: change params to allow host param
    SOCKET server = socket_create(hostname, 25, 1, AF_INET, SOCK_STREAM);

    // blocking, may want to poll instead or set timeout
    wait_on_response(server, 220);

    send_format(server, "HELO HONPWC\r\n");
    wait_on_response(server, 250);

    send_format(server, "MAIL FROM:<%s>\r\n", sender);
    wait_on_response(server, 250);

    send_format(server, "RCPT TO:<%s>\r\n", recipient);
    wait_on_response(server, 250);

    // going to need template instead
    send_format(server, "DATA\r\n");
    wait_on_response(server, 354);

    send_format(server, "subject: %s", "test");
    send_format(server, "From:<%s>\r\n", sender);
    send_format(server, "To:<%s>\r\b", recipient);
    send_format(server, "Subject:%s\r\n", "test");

    time_t timer;
    time(&timer);

    struct tm* timeinfo;
    timeinfo = gmtime(&timer);

    char date[128];
    strftime(date, 128, "%a, %d %b %Y %H:%M:%S +0000", timeinfo);

    send_format(server, "Date:%s\r\n", date);
    send_format(server, "\r\n");

    // add email regex
    printf("You email here...\n");
    send_format(server, "%s\r\n", "admin@quantizmo.com");

    send_format(server, "QUIT\r\n");
    wait_on_response(server, 221);    

    printf("Closing socket...\n");
    CLOSESOCKET(server);

#ifdef _WIN32
    WSACleanup();
#endif

    printf("Finished.\n");
    return 0;
}