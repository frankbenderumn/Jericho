#include "server/frame.h"

int ws_sendframe(Client* client, const char *message, uint64_t size, int type) {
	unsigned char *response; /* Response data.     */
	unsigned char frame[10]; /* Frame.             */
	uint8_t idx_first_rData; /* Index data.        */
	int idx_response;        /* Index response.    */
	ssize_t output;          /* Bytes sent.        */
	ssize_t send_ret;        /* Ret send function  */
	uint64_t i;              /* Loop index.        */
	Client* lclient;

	frame[0] = (FIN | type);
	uint64_t length = (uint64_t)size; // message length

	/* Split the size between octets. */
	if (length <= 125) {
		frame[1] = length & 0x7F;
		idx_first_rData = 2;
	}

	/* Size between 126 and 65535 bytes. */
	else if (length >= 126 && length <= 65535) {
		frame[1] = 126;
		frame[2] = (length >> 8) & 255;
		frame[3] = length & 255;
		idx_first_rData = 4;
	}

	/* More than 65535 bytes. */
	else {
		frame[1] = 127;
		frame[2] = (unsigned char)((length >> 56) & 255);
		frame[3] = (unsigned char)((length >> 48) & 255);
		frame[4] = (unsigned char)((length >> 40) & 255);
		frame[5] = (unsigned char)((length >> 32) & 255);
		frame[6] = (unsigned char)((length >> 24) & 255);
		frame[7] = (unsigned char)((length >> 16) & 255);
		frame[8] = (unsigned char)((length >> 8) & 255);
		frame[9] = (unsigned char)(length & 255);
		idx_first_rData = 10;
	}

	/* Add frame bytes. */
	idx_response = 0;
	response = (unsigned char*)malloc(sizeof(unsigned char) * (idx_first_rData + length + 1));
	if (!response)
		return (-1);

	for (i = 0; i < idx_first_rData; i++) {
		response[i] = frame[i];
		idx_response++;
	}

	/* Add data bytes. */
	for (i = 0; i < length; i++) {
		response[idx_response] = message[i];
		idx_response++;
	}

	response[idx_response] = '\0';

	// printf("FRAME RESPONSE: %s\n", (char*)response);

	/* Send to the client if there is one. */
	output = 0;
	if (client) output = broadcast(client, response, idx_response, 0);

	/* If no client specified, broadcast to everyone. */
	if (!client) {

		BRED("TUB: FIX FOR MULTIPLE THREADS\n");

	}
	
	free(response);
	return ((int)output);
}

int ws_sendframe_txt(Client *client, const char *message) {
	return ws_sendframe(client, message, (uint64_t)strlen(message), FRAME_TXT);
}

int ws_sendframe_bin(Client *client, const char *message, uint64_t size) {
	return ws_sendframe(client, message, size, FRAME_BIN);
}

static inline int is_control_frame(int frame) {
	return (frame == FRAME_CLOSE || frame == FRAME_PING || frame == FRAME_PONG);
}

static inline int next_byte(Frame *frame) {
	ssize_t n;

	// If empty or full
	if (frame->cur_pos == 0 || frame->cur_pos == frame->received) {
		// fd_set set;
        // FD_ZERO(&set);
        // FD_SET(frame->client->socket, &set);

        // struct timeval timeout;
        // timeout.tv_sec = 5; timeout.tv_usec = 0;
        // select(frame->client->socket+1, 0, &set, 0, &timeout);

		// int n = recv(frame->client->socket, frame->request, sizeof(frame->request), 0);
		// if (n <= 0) {
		// 	frame->error = 1;
		// 	PERR(EWS, "An error has occurred while trying to read next byte\n");
		// 	return (-1);
		// }
		frame->received = (size_t)(strlen(frame->client->request) + 1);
		printf("Next byte: %s\n", (char*)frame->request);
		printf("Frame received: %i\n", (int)frame->received);
		// frame->received = 0;
		// frame->cur_pos = 0;
	}
	return (frame->request[frame->cur_pos++]);
}

int skip_frame(Frame* frame, uint64_t frame_size) {
	uint64_t i;
	for (i = 0; i < frame_size; i++) {
		if (next_byte(frame) == -1) {
			frame->error = 1;
			return (-1);
		}
	}
	return (0);
}

int read_frame(Frame* frame, int opcode, unsigned char** buf, 
  uint64_t* frame_length, uint64_t* frame_size, uint64_t* message_idx, uint8_t* masks, int is_fin) {

	unsigned char* tmp; /* Tmp message.     */
	int cur_byte;       /* Curr byte read.  */
	uint64_t i;         /* Loop index.      */

	unsigned char* message = *buf; // current message

	/* Decode masks and length for 16-bit messages. */
	if (*frame_length == 126) *frame_length = (((uint64_t)next_byte(frame)) << 8) | next_byte(frame);

	/* 64-bit messages. */
	else if (*frame_length == 127) {
		*frame_length =
			(((uint64_t)next_byte(frame)) << 56) | /* frame[2]. */
			(((uint64_t)next_byte(frame)) << 48) | /* frame[3]. */
			(((uint64_t)next_byte(frame)) << 40) | (((uint64_t)next_byte(frame)) << 32) |
			(((uint64_t)next_byte(frame)) << 24) | (((uint64_t)next_byte(frame)) << 16) |
			(((uint64_t)next_byte(frame)) << 8) |
			(((uint64_t)next_byte(frame))); /* frame[9]. */
	}

	*frame_size += *frame_length;

	if (*frame_size > MAX_FRAME_LENGTH) {
		PERR(EWS, "Current frame from client %d, exceeds the maximum\n"
			  "amount of bytes allowed (%lu/%d)!",
			frame->client->socket, *frame_size + *frame_length, MAX_FRAME_LENGTH)
		frame->error = 1;
		return (-1);
	}

	// Read masks
	masks[0] = next_byte(frame);
	masks[1] = next_byte(frame);
	masks[2] = next_byte(frame);
	masks[3] = next_byte(frame);

	// Abort if error.
	if (frame->error) return (-1);

	// Allocate memory
	if (*frame_length > 0) {
		if (!is_control_frame(opcode)) {
			tmp = (unsigned char*)realloc(
				message, sizeof(unsigned char) * (*message_idx + *frame_length + is_fin));
			if (!tmp) {
				// CHECK FOR PRI64
				PERR(EWS, "Cannot allocate memory, requested: %lu\n",
					(*message_idx + *frame_length + is_fin));

				frame->error = 1;
				return (-1);
			}
			message = tmp;
			*buf = message;
		}

		/* Copy to the proper location. */
		for (i = 0; i < *frame_length; i++, (*message_idx)++) {
			/* We were able to read? .*/
			cur_byte = next_byte(frame);
			if (cur_byte == -1)
				return (-1);

			message[*message_idx] = cur_byte ^ masks[i % 4];
		}
	}

	/* If we're inside a FIN frame, lets... */
	if (is_fin && *frame_size > 0) {
		/* Increase memory if our FIN frame is of length 0. */
		if (!*frame_length && !is_control_frame(opcode)) {
			tmp = (unsigned char*)realloc(message, sizeof(unsigned char) * (*message_idx + 1));
			if (!tmp) {
				// DEBUG("Cannot allocate memory, requested: %" PRId64 "\n",
				// 	(*message_idx + 1));

				frame->error = 1;
				return (-1);
			}
			message = tmp;
			*buf = message;
		}
		message[*message_idx] = '\0';
	}

	return (0);
}

int next_frame(Frame* frame) {
	uint8_t masks_data[4];   /* Masks data frame array.    */
	uint8_t masks_ctrl[4];   /* Masks control frame array. */
	uint32_t utf8_state;     /* Current UTF-8 state.       */
	int32_t pong_id;         /* Current PONG id.           */
	uint8_t opcode;          /* Frame opcode.              */
	uint8_t mask;            /* Mask.                      */
	int cur_byte;            /* Current frame byte.        */

	unsigned char* message_data = NULL;         // data frame
	unsigned char* payload = frame->payload;    // control frame
	uint8_t is_fin = 0;                         // FIN frame flag
	uint64_t frame_length = 0;                   // Frame length
	uint64_t frame_size = 0;                    // Current frame size
	uint64_t message_idx_data = 0;              // Current message index
	uint64_t message_idx_ctrl = 0;              // Current message index
	frame->size = 0; 
	frame->type = -1;
	frame->message = NULL;
	utf8_state = UTF8_ACCEPT;

	/* Read until find a FIN or a unsupported frame. */
	do {
		cur_byte = next_byte(frame);
		printf("Next byte\n");
		if (cur_byte == -1) { PERR(ESERVER, "Can't read next byte"); return -1; }

		is_fin = (cur_byte & 0xFF) >> FRAME_FIN;
		opcode = (cur_byte & 0xF);

		// Check for RSV field.
		if (cur_byte & 0x70) {
			PERR(ESERVER, "RSV is set while wsServer do not negotiate extensions!\n");
			frame->error = 1;
			break;
		}

		if ((frame->type == -1 && opcode == FRAME_CONT) ||
			(frame->type != -1 && !is_control_frame(opcode) && opcode != FRAME_CONT)) {
			// PERR(ESERVER, "Unexpected frame was received!, opcode: %d, previous: %d\n", opcode, frame->type);
			frame->error = 1;
			break;
		}

		/* Check if one of the valid opcodes. */
		if (opcode == FRAME_TXT || opcode == FRAME_BIN ||
			opcode == FRAME_CONT || opcode == FRAME_PING ||
			opcode == FRAME_PONG || opcode == FRAME_CLOSE) {

			if (client_get_state(frame->client) == SOCKST_CLOSING &&
				opcode != FRAME_CLOSE) {
				DEBUG("Unexpected frame received, expected CLOSE (%d), "
					  "received: (%d)",
					FRAME_CLOSE, opcode);
				frame->error = 1;
				break;
			}

			// Only change frame type if not a CONT frame.
			if (opcode != FRAME_CONT && !is_control_frame(opcode)) frame->type = opcode;

			mask = next_byte(frame);
			frame_length = mask & 0x7F;
			frame_size = 0;
			message_idx_ctrl = 0;

			
			// We should deny non-FIN control frames or that have more than 125 octets.
			if (is_control_frame(opcode) && (!is_fin || frame_length > 125)) {
				PERR(ESERVER, "Control frame bigger than 125 octets or not a FIN frame!\n");
				frame->error = 1;
				break;
			}

			// Normal data frames.
			if (opcode == FRAME_TXT || opcode == FRAME_BIN ||
				opcode == FRAME_CONT) {
				if (read_frame(frame, opcode, &message_data, &frame_length,
						&frame->size, &message_idx_data, masks_data, is_fin) < 0)
					break;

#ifdef VALIDATE_UTF8
				// /* UTF-8 Validate partial (or not) frame. */
				if (frame->type == FRAME_TXT)
				{
					if (is_fin)
					{
						if (is_utf8_len_state(
								message_data + (message_idx_data - frame_length),
								frame_length, utf8_state) != UTF8_ACCEPT)
						{
							DEBUG("Dropping invalid complete message!\n");
							frame->error = 1;
							do_close(frame, WS_CLSE_INVUTF8);
						}
					}

					/* Check current state for a CONT or initial TXT frame. */
					else
					{
						utf8_state = is_utf8_len_state(
							message_data + (message_idx_data - frame_length), frame_length,
							utf8_state);

						/* We can be in any state, except reject. */
						if (utf8_state == UTF8_REJECT)
						{
							DEBUG("Dropping invalid cont/initial frame!\n");
							frame->error = 1;
							do_close(frame, WS_CLSE_INVUTF8);
						}
					}
				}
#endif
			}

			else if (opcode == FRAME_PONG) {
				if (read_frame(frame, opcode, &payload, &frame_length, &frame_size,
						&message_idx_ctrl, masks_ctrl, is_fin) < 0)
					break;

				is_fin = 0;

				// If there is no content and/or differs the size, ignore it.
				if (frame_size != sizeof(frame->client->last_pong_id)) continue;

				pthread_mutex_lock(&frame->client->ping_mutex);

					pong_id = pong_message_to_int32(payload);
					if (pong_id < 0 || pong_id > frame->client->current_ping_id) {
						pthread_mutex_unlock(&frame->client->ping_mutex);
						continue;
					}
					frame->client->last_pong_id = pong_id;

				pthread_mutex_unlock(&frame->client->ping_mutex);
				/* clang-format on */
				continue;
			}

			/* We should answer to a PING frame as soon as possible. */
			else if (opcode == FRAME_PING) {
				if (read_frame(frame, opcode, &payload, &frame_length, &frame_size,
						&message_idx_ctrl, masks_ctrl, is_fin) < 0)
					break;

				if (do_pong(frame, frame_size) < 0)
					break;

				/* Quick hack to keep our loop. */
				is_fin = 0;
			}

			/* We interrupt the loop as soon as we find a CLOSE frame. */
			else
			{
				if (read_frame(frame, opcode, &payload, &frame_length, &frame_size,
						&message_idx_ctrl, masks_ctrl, is_fin) < 0)
					break;

#ifdef VALIDATE_UTF8
				// /* If there is a close reason, check if it is UTF-8 valid. */
				// if (frame_size > 2 && !is_utf8_len(payload + 2, frame_size - 2))
				// {
				// 	PERR(ESERVER, "Invalid close frame payload reason! (not UTF-8)\n");
				// 	frame->error = 1;
				// 	break;
				// }
#endif

				/* Since we're aborting, we can scratch the 'data'-related
				 * vars here. */
				frame->size = frame_size;
				frame->type = FRAME_CLOSE;
				free(message_data);
				return 0;
			}
		}

		/* Anything else (unsupported frames). */
		else {
            char buffer[64];
			PERR(ESERVER, "Unsupported frame opcode: %d\n", opcode);
			// We should consider as error receive an unknown frame.
			frame->type = opcode;
			frame->error = 1;
		}

	} while (!is_fin && !frame->error);

	// Check for error.
	if (frame->error) {
		free(message_data);
		frame->message = NULL;
		return -1;
	}

	frame->message = message_data;
	return 0;
}