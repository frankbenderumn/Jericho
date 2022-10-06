#include "server/web_socket.h"

extern EventManager event_manager;

int handshake_key(char *wsKey, unsigned char **dest)
{
	unsigned char hash[SHA1HashSize]; /* SHA-1 Hash.                   */
	SHA1Context ctx;                  /* SHA-1 Context.                */
	char *str;                        /* WebSocket key + magic string. */

	/* Invalid key. */
	if (!wsKey) return -1;

	str = (char*)calloc(1, sizeof(char) * (KEY_LEN + MAGIC_STRING_LEN + 1));
	if (!str) return -1;

	strncpy(str, wsKey, KEY_LEN);
	strcat(str, MAGIC_STRING);

	SHA1Reset(&ctx);
	SHA1Input(&ctx, (const uint8_t *)str, PLAIN_LEN);
	SHA1Result(&ctx, hash);

	*dest = base64_encode(hash, SHA1HashSize, NULL);
	*(*dest + strlen((const char *)*dest) - 1) = '\0';
	free(str);
	return 0;
}

/**
 * @brief Gets the complete response to accomplish a succesfully
 * handshake.
 *
 * @param hsrequest  Client request.
 * @param hsresponse Server response.
 *
 * @return Returns 0 if success and a negative number
 * otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */

int handshake(char *hsrequest, char **hsresponse)
{
	BCYA("Handshaking\n");
	unsigned char *accept; /* Accept message.     */
	char *saveptr;         /* strtok_r() pointer. */
	char *s;               /* Current string.     */
	int ret;               /* Return value.       */

	saveptr = NULL;
	for (s = strtok_r(hsrequest, "\r\n", &saveptr); s != NULL;
		 s = strtok_r(NULL, "\r\n", &saveptr)) {
		if (strstr(s, WS_REQUEST) != NULL) break;
	}

	/* Ensure that we have a valid pointer. */
	if (s == NULL) return (-1);

	saveptr = NULL;
	s       = strtok_r(s, " ", &saveptr);
	s       = strtok_r(NULL, " ", &saveptr);

	ret = handshake_key(s, &accept);
	if (ret < 0)
		return (ret);

	*hsresponse = (char*)malloc(sizeof(char) * ACCEPT_LEN);
	if (*hsresponse == NULL) return -1;

	strcpy(*hsresponse, WS_HEADER);
	strcat(*hsresponse, (const char *)accept);
	strcat(*hsresponse, "\r\n\r\n");

	free(accept);
	return 0;
}



int upgrade(Frame* frame) {
    char* response;
    unsigned char* accept; // Accept message.
	char* saveptr = NULL;         // strtok_r() pointer.
	char* s;               // Current string.

	for (s = strtok_r((char*)frame->request, "\r\n", &saveptr); s != NULL; s = strtok_r(NULL, "\r\n", &saveptr)) {
		if (strstr(s, WS_HEADER) != NULL) break;
	}

	/* Ensure that we have a valid pointer. */
	if (s == NULL) { 
        PERR(EWS, "Invalid pointer on request parse"); 
        return 0; 
    }

	saveptr = NULL;
	s       = strtok_r(s, " ", &saveptr);
	s       = strtok_r(NULL, " ", &saveptr);

///////////////////////////////////////////////////////////////////////////////////////////////////

    // ACCEPT HANDSHAKE CODE

	int retVal = handshake_key(s, &accept);
	if (retVal < 0) { PERR(ESERVER, "Failed to accept handshake"); }


///////////////////////////////////////////////////////////////////////////////////////////////////

    BBLU("Accept key is: %s\n", (char*)accept);
    BLU("Size of is: %i\n", (int)strlen((char*)accept));

	response = (char*)malloc(sizeof(char) * ACCEPT_LEN);
	if (response == NULL) {
        printf("Failed request: %s\n", (const char*)frame->request);
        PERR(ESERVER, "Cannot get handshake response");
        return -1;
    }

	strcpy(response, "HTTP/1.1 101 Switching Protocols\r\n"
                            "Upgrade: websocket\r\n"
                            "Connection: Upgrade\r\n"
                            "Sec-WebSocket-Accept: ");
	strcat(response, (const char *)accept);
	strcat(response, "\r\n\r\n");

	free(accept);
	// return 0;

///////////////////////////////////////////////////////////////////////////////////////////////////

	/* Valid request. */
	printf("Handshaked, response: \n"
		  "------------------------------------\n"
            "%s"
		  "------------------------------------\n",
		response);

    // strcat(response, "sUBSUSusush");

    printf("RESPONSE SIZE IS: %i", (int)strlen(response));

	/* Send handshake. */
	if (broadcast(frame->client, response, strlen(response), 0) < 0) {
		free(response);
		PERR(ESERVER, "As error has occurred while handshaking!\n");
		return -1;
	}

    // do_pong(frame, strlen(frame->payload), FRAM)

	// Trigger events and clean up buffers
    GRE("YIPPEE KI YAE!\n");
	event_manager.open(frame->client);
	free(response);
	return 0;
}

	// char *response; /* Handshake response message. */
	// char *p;        /* Last request line pointer.  */
	// ssize_t n;      /* Read/Write bytes.           */

	// /* Read the very first client message. */
	// if ((n = RECV(wfd->client, wfd->frm, sizeof(wfd->frm) - 1)) < 0)
	// 	return (-1);

	// /* Advance our pointers before the first next_byte(). */
	// p = strstr((const char *)wfd->frm, "\r\n\r\n");
	// if (p == NULL)
	// {
	// 	DEBUG("An empty line with \\r\\n was expected!\n");
	// 	return (-1);
	// }
	// wfd->amt_read = n;
	// wfd->cur_pos = (size_t)((ptrdiff_t)(p - (char *)wfd->frm)) + 4;

	// /* Get response. */
	// if (get_handshake_response((char *)wfd->frm, &response) < 0)
	// {
	// 	DEBUG("Cannot get handshake response, request was: %s\n", wfd->frm);
	// 	return (-1);
	// }

	// /* Valid request. */
	// DEBUG("Handshaked, response: \n"
	// 	  "------------------------------------\n"
	// 	  "%s"
	// 	  "------------------------------------\n",
	// 	response);

	// /* Send handshake. */
	// if (SEND(wfd->client, response, strlen(response)) < 0)
	// {
	// 	free(response);
	// 	DEBUG("As error has occurred while handshaking!\n");
	// 	return (-1);
	// }

	// /* Trigger events and clean up buffers. */
	// cli_events.onopen(wfd->client);
	// free(response);
	// return (0);