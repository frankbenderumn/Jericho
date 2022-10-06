// #include "../include/server.h"

// char* cread(const char* path) {
//     FILE* fp;
//     char* buffer = 0;
//     fp = fopen(path, "a+");
//     fseek(fp, 0L, SEEK_END);
//     long sz = ftell(fp);
//     rewind(fp);
//     if (sz > 50) {
//         printf("Buffer overflow risk, file size is: %li\n", sz);
//     }
//     if (NULL == fp) {
//         printf("file can't be opened \n");
//         exit(1);
//     }
 
//     printf("content of this file are: \n");
 
//     buffer = (char*)malloc(sz);
//     if (buffer) {
//         fread (buffer, 1, sz, fp);
//     }

//     if (buffer == NULL) {
//         printf("Buffer is empty\n");
//     }

//     printf("BUFFER: %s", buffer);
//     fclose(fp);

//     return buffer;
// }

// void ping() {
//     printf("WELL GOD DAM IT\n");
// }

// int main() {

// #ifdef _WIN32
//     WSAData d;
//     if (WSAStartup(MAKEWORD(2, 2), &d)) {
//         fprintf(stderr, "Failed to initialize.\n");
//         return 1;
//     }
// #endif

//     initOpenSSL();

//     SSL_CTX* ctx = getCertificate();

//     printf("Configuring local address...\n");
//     struct addrinfo hints;
//     memset(&hints, 0, sizeof(hints));
//     hints.ai_family = AF_INET;
//     hints.ai_socktype = SOCK_STREAM;
//     hints.ai_flags = AI_PASSIVE;

//     struct addrinfo* bind_address;
//     getaddrinfo(0, "8081", &hints, &bind_address);

//     printf("Creating socket...\n");
//     SOCKET socket_listen;
//     socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

//     int enable = 1;
//     if (setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
//         // PFAIL(ESERVER, "REUSE SOCKET ERROR.");
//     }

//     if (!ISVALIDSOCKET(socket_listen)) {
//         // fprintf(stderr, "socket failed(). (%d)\n", SOCKERR());
//         // PERR(ESERVER, "socket() failed.");
//         return 1;
//     }

//     printf("Binding socket to local address...\n");
//     if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
//         // fprintf(stderr, "bind() failed. (%d)\n", SOCKERR());
//         // PERR(ESERVER, "bind() failed.");
//         return 1;
//     }
//     freeaddrinfo(bind_address);

//     printf("Listening...\n");
//     if (listen(socket_listen, 10) < 0) {
//         // fprintf(stderr, "listen() failed. (%d)\n", SOCKERR());
//         // PERR(ESERVER, "listen() failed.");
//         return 1;
//     }

//     while(1) {
//         printf("Waiting for connection...\n");
//         struct sockaddr_storage client_address;
//         socklen_t client_len = sizeof(client_address);
//         SOCKET socket_client = accept(socket_listen, (struct sockaddr*)&client_address, &client_len);
//         if (!ISVALIDSOCKET(socket_client)) {
//             // fprintf(stderr, "accept() failed. (%d)\n", SOCKERR());
//             // PERR(ESERVER, "accept() failed.");
//             return 1;
//         }

//         printf("Client is connected... ");
//         char address_buffer[100];
//         getnameinfo((struct sockaddr*)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
//         printf("%s\n", address_buffer);

//         SSL* ssl = SSL_new(ctx);
//         if (!ctx) {
//             fprintf(stderr, "SSL_new() failed.\n");
//         }

//         SSL_set_fd(ssl, socket_client);
//         if (SSL_accept(ssl) <= 0) {
//             // PWARN(ESERVER, "SSL_accept() failed.\n");
//             ERR_print_errors_fp(stderr);

//             SSL_shutdown(ssl);
//             CLOSESOCKET(socket_client);
//             SSL_free(ssl);    

//             continue;
//         }

//         // printf("SSL connection using %s\n", SSL_get_cipher(ssl));

//         printf("Reading request...\n");
//         char request[1024];
//         int bytes_received = receiveRequest(ssl, request);
//         printf("Received %d bytes.\n", bytes_received);

//         printf("Sending response...\n");
//         char* response = "HTTP/1.0 200 OK\r\n"
//                                 "Connection: close\r\n"
//                                 "Content-Type: text/html charset=UTF-8\r\n\r\n"
//                                 "Local time is: ";
//         int bytes_sent = sendRequest(ssl, response);
//         // SSL_write(ssl, response, strlen(response));
//         printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(response));

//         // time_t timer;
//         // time(&timer);
//         // char* time_msg = ctime(&timer);
//         // bytes_sent = SSL_write(ssl, time_msg, strlen(time_msg));
        
//         // send html file
//         char* header = "HTTP/1.0 200 OK\r\n"
//                         "Connection: close\r\n"
//                         "Content-Type: text/plain\r\n\r\n";
//         char* buf = cread("./frontend/index.html");
//         int newSz = strlen(buf) + strlen(header) + 1; 
//         char* newBuf = (char*)malloc(newSz);
//         strcpy(newBuf,header);
//         strcat(newBuf,buf); // or strncat
//         // free(header);
//         // free(buf);
//         // bytes_sent = SSL_write(ssl, buf, strlen(buf));
//         // printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(time_msg));
//         free(newBuf);
//         printf("Closing connection...\n");
//         SSL_shutdown(ssl);
//         CLOSESOCKET(socket_client);
//         SSL_free(ssl);
//     }

//     printf("Closing listening socket...\n");
//     CLOSESOCKET(socket_listen); 
//     SSL_CTX_free(ctx);

// #ifdef _WIN32
//     WSACleanup();
// #endif

//     printf("Finished.\n");

//     return 0;
// }