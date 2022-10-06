// #include "server/dumb_demo.h"

// static struct client_info *clients_arr = 0;

// int main() {

// #if defined(_WIN32)
//     WSADATA d;
//     if (WSAStartup(MAKEWORD(2, 2), &d)) {
//         fprintf(stderr, "Failed to initialize.\n");
//         return 1;
//     }
// #endif



//     SSL_library_init();
//     OpenSSL_add_all_algorithms();
//     SSL_load_error_strings();

//     SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
//     if (!ctx) {
//         fprintf(stderr, "SSL_CTX_new() failed.\n");
//         return 1;
//     }


//     if (!SSL_CTX_use_certificate_file(ctx, "cert.pem" , SSL_FILETYPE_PEM)
//     || !SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM)) {
//         fprintf(stderr, "SSL_CTX_use_certificate_file() failed.\n");
//         ERR_print_errors_fp(stderr);
//         return 1;
//     }


//     SOCKET server = create_socket(0, "8080");


//     while(1) {

//         fd_set reads;
//         reads = wait_on_clients(server, &clients_arr);

//         if (FD_ISSET(server, &reads)) {
//             struct client_info *client = get_client(-1, &clients_arr);

//             client->socket = accept(server,
//                     (struct sockaddr*) &(client->address),
//                     &(client->address_length));

//             if (!ISVALIDSOCKET(client->socket)) {
//                 fprintf(stderr, "accept() failed. (%d)\n",
//                         SOCKERR());
//                 return 1;
//             }


//             client->ssl = SSL_new(ctx);
//             if (!client->ssl) {
//                 fprintf(stderr, "SSL_new() failed.\n");
//                 return 1;
//             }

//             SSL_set_fd(client->ssl, client->socket);
//             if (SSL_accept(client->ssl) != 1) {
//                 //SSL_get_error(client->ssl, SSL_accept(...));
//                 ERR_print_errors_fp(stderr);
//                 drop_client(client, &clients_arr);
//             } else {
//                 printf("New connection from %s.\n",
//                         get_client_address(client));

//                 printf ("SSL connection using %s\n",
//                         SSL_get_cipher(client->ssl));
//             }
//         }


//         struct client_info *client = clients_arr;
//         while(client) {
//             BRED("BOOYAH\n");
//             struct client_info *next = client->next;

//             if (FD_ISSET(client->socket, &reads)) {

//                 if (MAX_REQUEST_SIZE == client->received) {
//                     send_400(client, &clients_arr);
//                     client = next;
//                     continue;
//                 }

//                 int r = SSL_read(client->ssl,
//                         client->request + client->received,
//                         MAX_REQUEST_SIZE - client->received);

//                 if (r < 1) {
//                     printf("Unexpected disconnect from %s.\n",
//                             get_client_address(client));
//                     drop_client(client, &clients_arr);

//                 } else {
//                     client->received += r;
//                     client->request[client->received] = 0;

//                     char *q = strstr(client->request, "\r\n\r\n");
//                     if (q) {
//                         *q = 0;

//                         if (strncmp("GET /", client->request, 5)) {
//                             send_400(client, &clients_arr);
//                         } else {
//                             char *path = client->request + 4;
//                             char *end_path = strstr(path, " ");
//                             if (!end_path) {
//                                 send_400(client, &clients_arr);
//                             } else {
//                                 *end_path = 0;
//                                 serve_resource(client, path, &clients_arr);
//                             }
//                         }
//                     } //if (q)
//                 }
//             }

//             client = next;
//         }

//     } //while(1)


//     printf("\nClosing socket...\n");
//     CLOSESOCKET(server);
//     SSL_CTX_free(ctx);

// #if defined(_WIN32)
//     WSACleanup();
// #endif

//     printf("Finished.\n");
//     return 0;
// }

