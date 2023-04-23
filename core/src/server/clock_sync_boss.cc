#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <syslog.h>
#include <ntp/ntp.h>

#define NTP_PORT 123

int main() {
    // Initialize the NTP library
    if (ntp_init() < 0) {
        printf("Error initializing NTP library: %s\n", ntp_strerror(errno));
        return 1;
    }

    // Create a socket to receive NTP requests
    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd < 0) {
        perror("socket");
        return 1;
    }

    // Bind the socket to the NTP port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NTP_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock_fd);
        return 1;
    }

    // Set the socket to non-blocking mode
    if (fcntl(sock_fd, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl");
        close(sock_fd);
        return 1;
    }

    // Loop to handle NTP requests
    while (1) {
        // Wait for an NTP request using select
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock_fd, &readfds);
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int retval = select(sock_fd + 1, &readfds, NULL, NULL, &tv);
        if (retval < 0) {
            perror("select");
            continue;
        } else if (retval == 0) {
            // Timeout
            continue;
        }

        // Read the NTP request from the socket
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        char buf[512];
        int bytes_received = recvfrom(sock_fd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &client_len);
        if (bytes_received < 0) {
            perror("recvfrom");
            continue;
        }

        // Process the NTP request and send the response
        struct ntptimeval tv;
        ntp_gettime(&tv);
        struct ntpdata ntp;
        memset(&ntp, 0, sizeof(ntp));
        ntp_status_to_timedata(&tv.time, &ntp);
        ntp.tsf = tv.maxerror << 16;
        ntp_stratum(&ntp) = 1;
        ntp_mode(&ntp) = NTP_MODE_SERVER;
        ssize_t bytes_sent = sendto(sock_fd, &ntp, sizeof(ntp), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        if (bytes_sent < 0) {
            perror("sendto");
            continue;
        }
    }

    // Close the socket and deinitialize the NTP library
    close(sock_fd);
    ntp_shutdown();
    return 0;
}