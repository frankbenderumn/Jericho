#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main() {
    // Create a non-blocking socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock_fd, F_SETFL, O_NONBLOCK);

    // Connect to the server using non-blocking connect
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "www.example.com", &addr.sin_addr);
    connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr));

    // Wait for the socket to become writable using select
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sock_fd, &writefds);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    int retval = select(sock_fd + 1, NULL, &writefds, NULL, &tv);

    // Check if the socket is ready for writing
    if (retval == 1 && FD_ISSET(sock_fd, &writefds)) {
        // Send the data using non-blocking send
        const char *data = "GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
        int data_len = strlen(data);
        int bytes_sent = send(sock_fd, data, data_len, MSG_DONTWAIT);
        if (bytes_sent == data_len) {
            printf("Data sent successfully!\n");
        } else {
            printf("Error sending data: %s\n", strerror(errno));
        }
    } else {
        printf("Error connecting to server: %s\n", strerror(errno));
    }

    // Close the socket
    close(sock_fd);

    return 0;
}