#include "server/socket.h"
#include <sys/ioctl.h>


void socket_close(int fd) {
#ifndef _WIN32
	shutdown(fd, SHUT_RDWR);
	close(fd);
#else
	closesocket(fd);
#endif
}

extern EventManager event_manager;

SOCKET socket_create(const char* host, int port, int reuse, int family, int socktype) {
    YEL("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // use AF_INET6 for IPv6 (websockets dont work with IPv6)
    hints.ai_socktype = SOCK_STREAM; // type of socket (use DGRAM for UDP)
    hints.ai_flags = AI_PASSIVE; // idk, something related to allowing the socket bind to be ignored if NULL

    struct addrinfo* bind_address;
    char cport[7];
    snprintf(cport, 7, "%i", port);
    getaddrinfo(host, cport, &hints, &bind_address);  // address to bind to

    // create the socket
    printf("Creating socket...\n");
    SOCKET sock = socket(bind_address->ai_family, bind_address->ai_socktype, 
                                    bind_address->ai_protocol);

    // error check in socket creation
    if (!ISVALIDSOCKET(sock)) {
        PFAIL(ESERVER, "socket() failed."); // maybe change for vargs preprocess PFAIL(...) __VA_ARGS__
    }

    // allow for development and reusable connection
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
        PFAIL(ESERVER, "setsockopt() with resuse flag failed.");
    }

    // for receive timeout if desired
    // struct timeval tv;
    // tv.tv_sec = 3;
    // tv.tv_usec = 0;
    // if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
    //     PFAIL(ESERVER, "setsockopt() timeout failed.");
    // }

    // sets socket to non blocking
    // int i = 1;
    // if (ioctl(sock, FIONBIO, (char*)&i) < 0) {
    //     PFAIL(ESERVER, "ioctl() failed.");
    // }

    //  bind the socket to local address
    printf("Binding socket to local address...\n");
    if (bind(sock, bind_address->ai_addr, bind_address->ai_addrlen)) {
        PFAIL(ESERVER, "bind() failed."); // maybe change for vargs preprocess PFAIL(...) __VA_ARGS__
    }
    freeaddrinfo(bind_address);

    // set to listen for a conn
    printf("Listening...\n");
    if (listen(sock, 10) < 0) {
        PFAIL(ESERVER, "listen() failed with thread_pool 5000");
    }
    clearcolor();

    // YEL("Adding socket to server set\n");
    // /* Add the socket to the server's set of active sockets */
    // FD_SET(server_sockets[0], &server_read_set);
    // server_sockets_active++;

    // server_comm_created = TRUE;
    return sock;
}

const char* get_content_type(const char* path) {
    const char* last_dot = strrchr(path, '.');
    if (last_dot) {
        if (strcmp(last_dot, ".css") == 0) return "text/css";
        if (strcmp(last_dot, ".csv") == 0) return "text/csv";
        if (strcmp(last_dot, ".gif") == 0) return "text/gif";
        if (strcmp(last_dot, ".htm") == 0) return "text/htm";
        if (strcmp(last_dot, ".iris") == 0) return "text/html";
        if (strcmp(last_dot, ".html") == 0) return "text/html";
        if (strcmp(last_dot, ".ico") == 0) return "image/x-icon";
        if (strcmp(last_dot, ".jpeg") == 0) return "image/jpeg";
        if (strcmp(last_dot, ".jpg") == 0) return "image/jpeg";
        if (strcmp(last_dot, ".js") == 0) return "application/javascript";
        if (strcmp(last_dot, ".json") == 0) return "application/json";
        if (strcmp(last_dot, ".png") == 0) return "image/png";
        if (strcmp(last_dot, ".pdf") == 0) return "application/pdf";
        if (strcmp(last_dot, ".svg") == 0) return "image/svg+xml";
        if (strcmp(last_dot, ".txt") == 0) return "text/plain";
    }

    return "application/octet-stream";
}

// encrypts the key to authenticate websocket handshake
char* socket_key(const char* subkey) {
    // test subkey is dGhlIHNhbXBsZSBub25jZQ==
    char bufkey[strlen(subkey)];
    strncpy(bufkey, subkey, strlen(subkey) + 1);
    purple();
    printf("Bufkey is: %s\n", bufkey);
    char* key = bufkey; // key from client
    const char* magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; // magic string
    strcat(key, magic);
    printf("key is: %s\n", key);
    char digest[512]; 
    // sha1(digest, key, strlen(key));

    // from github
    #define WS_KEY_LEN     24
	#define WS_MS_LEN      36
	#define WS_KEYMS_LEN   (WS_KEY_LEN + WS_MS_LEN)
	// unsigned char hash[SHA1HashSize]; /* SHA-1 Hash. */
	// SHA1Context ctx;  
	// SHA1Reset(&ctx);
	// SHA1Input(&ctx, (const uint8_t *)digest, WS_KEYMS_LEN);
	// SHA1Result(&ctx, hash);

    // // hello world
    // char shaTest1[512];
    // char shaTest2[512];
    // const char* ex1 = "hello world";
    // // SHA1(hello world) -> 2aae6c35c94fcfb415dbe95f408b9ce91ee846ed
    // // SHA1(GeeksForGeeks) -> addf120b430021c36c232c99ef8d926aea2acd6b
    // printf("Digest is: %u\n", hash);
    // clearcolor();
    // const char* base_digest = digest;
    const char* base_digest = "Hello World!";
    // char* result = b64_encode(hash, strlen((char*)hash));
    unsigned char* dest;
    unsigned char hash[SHA1HashSize];
	SHA1Context ctx;                  
	char *str;    

	str = (char*)calloc(1, sizeof(char) * (WS_KEY_LEN + WS_MS_LEN + 1));
	strncpy(str, key, WS_KEY_LEN);
	strcat(str, magic);

	SHA1Reset(&ctx);
	SHA1Input(&ctx, (const uint8_t *)str, WS_KEYMS_LEN);
	SHA1Result(&ctx, hash); // hash (key + magic string)

	dest = base64_encode(hash, SHA1HashSize, NULL); // encode hash
	// (dest + strlen((const char *)dest) - 1) = '\0';
	free(str);
    
    static char result[24];
    strcpy(result, (char*)dest);

    // test cases
    const char* goal = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=";
    // const char* magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    // char* ext = b64_encode((const unsigned char*)"258EAFA5-E914-47DA-95CA-C5AB0DC85B11", strlen(base_digest));
    // yellow(); clearcolor();
    purple(); printf("GOAL: %s\n", goal); clearcolor();
    purple(); printf("BASE_DIGEST: %s\n", result); clearcolor();
    // if (strcmp(base_digest, goal) == 0) {
    //     bold(); green();
    //     printf("SUCCESS!!!!!\n");
    //     clearcolor();
    // }
    return result;
}

void handshake_response(Client* client, const char* path, const char* subkey) {
    const char* key = socket_key(subkey);
    // static test cases
    printf("COMPARE: %s = %s\n", key, "SGVsbG8gV29ybGQh");
    printf("COMPARE: %s = %s\n", key, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
    // Hello World! ->(Base64) SGVsbG8gV29ybGQh
    // 2nd one is for magic string websocket handshake
    char* buf = (char*)calloc(1, 75 + 50);
    const char* handshake = "HTTP/1.1 101 Switching Protocols\r\n"
                            "Upgrade: websocket\r\n"
                            "connection: Upgrade\r\n"
                            "Sec-WebSocket-Accept: ";

    strncpy(buf, handshake, strlen(handshake));
    strcat(buf, key);
    strcat(buf, "\r\n\r\n");

    if (send(client->socket, buf, strlen(buf), 0) == -1) {
        PERR(ESERVER, "Failed to send data in handshake");
    }
    blue(); printf("SENT: %s", buf); clearcolor();
    free(buf);
}

ssize_t broadcast(Client *client, const void *buf, size_t len, int flags) {
	const char *p;
	ssize_t ret;

	if (!ISVALIDSOCKET(client)) { PERR(ESERVER, "Trying to send invalid socket"); return -1; }

	p = (const char*)buf;

	pthread_mutex_lock(&client->send_mutex);
		while (len) {
			ret = SSL_write(client->ssl, p, len);
			if (ret == -1) {
                PERR(ESERVER, "Nothing sent to client");
				pthread_mutex_unlock(&client->send_mutex);
				return -1;
			}
			p += ret;
			len -= ret;
		}
	pthread_mutex_unlock(&client->send_mutex);

	return 0;
}
