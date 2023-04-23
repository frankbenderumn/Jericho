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

int main() {
    // Initialize the NTP library
    if (ntp_init() < 0) {
        printf("Error initializing NTP library: %s\n", ntp_strerror(errno));
        return 1;
    }

    // Create a socket to send NTP requests
    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd < 0) {
        printf("Error creating socket: %s\n", strerror(errno));
        return 1;
    }

    // Set the socket to non-blocking mode
    if (fcntl(sock_fd, F_SETFL, O_NONBLOCK) < 0) {
        printf("Error setting socket to non-blocking mode: %s\n", strerror(errno));
        close(sock_fd);
        return 1;
    }

    // Configure the NTP server address and port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NTP_PORT);
    inet_pton(AF_INET, "192.168.0.1", &(addr.sin_addr));

    // Loop to synchronize the clock with the NTP server
    while (1) {
        // Send an NTP request to the server
        struct ntpdata ntp;
        memset(&ntp, 0, sizeof(ntp));
        ntp_mode(&ntp) = NTP_MODE_CLIENT;
        if (ntp_tx(sock_fd, (struct sockaddr *)&addr, &ntp, sizeof(ntp)) < 0) {
            printf("Error sending NTP request: %s\n", strerror(errno));
            continue;
        }

        // Wait for the NTP response using select
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock_fd, &readfds);
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int retval = select(sock_fd + 1, &readfds, NULL, NULL, &tv);
        if (retval < 0) {
            printf("Error waiting for NTP response: %s\n", strerror(errno));
            continue;
        } else if (retval == 0) {
            // Timeout
            continue;
        }

        // Read the NTP response from the socket
        struct sockaddr_in server_addr;
        socklen_t server_len = sizeof(server_addr);
        char buf[512];
        int bytes_received = recvfrom(sock_fd, buf, sizeof(buf), 0, (struct sockaddr *)&server_addr, &server_len);
        if (bytes_received < 0) {
            printf("Error receiving NTP response: %s\n", strerror(errno));
            continue;
        }

        // Process the NTP response and adjust the clock
        struct ntptimeval tv;
        ntp_gettime(&tv);
        struct ntpdata *recv_ntp = (struct ntpdata *)buf;
        struct ntptimeval org, rec, xmt;
        ntp_to_timeval(ntp_xmit(recv_ntp), &xmt);
        ntp_to_timeval(ntp_org(recv_ntp), &org);
        ntp_to_timeval(ntp_rec(recv_ntp), &rec);
        tv.time.tv_sec += (rec.tv_sec - org.tv_sec + xmt.tv_sec - tv.time.tv_sec) / 2;
        tv.time.tv_usec += (rec.tv_usec - org.tv_usec + xmt.tv_usec - tv.time.tv_usec) / 2;
        ntp_adjtime(&tv);
        printf("Clock synchronized with NTP server\n");

        // Sleep for a period of time before sending the next request
        sleep(60);
    }

    // Close the socket and deinitialize the NTP library
    close(sock_fd);
    ntp_shutdown();
    return 0;
}