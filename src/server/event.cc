#include "server/event.h"

EventManager event_manager;

// implentation specifc
// void onopen(Conn* conn) {
//     printf("Opening");
// 	char buffer[16];
// 	conn_get_address(conn, buffer);
// 	printf("Connection opened, addr: %s\n", buffer);
// }

// void onclose(Conn* conn) {
// 	char buffer[16];
// 	conn_get_address(conn, buffer);
// 	printf("Connection closed, addr: %s\n", buffer);
// }

// void onmessage(Conn* conn, const unsigned char* message, uint64_t size, int type) {
// 	char buffer[16];
// 	conn_get_address(conn, buffer);
// 	printf("I receive a message: %s (size: %ld, type: %d), from: %s\n", message, size, type, buffer);
// 	// sendframe(NULL, (char *)message, size, type);
// }