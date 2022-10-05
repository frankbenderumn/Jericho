#include "server/shutdown.h"

static void *close_timeout(void *p) {
	Client *client = (Client*)p;
	struct timespec ts;
	int state;

	pthread_mutex_lock(&client->state_mutex);

	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_nsec += MS_TO_NS(TIMEOUT_MS);

	/* Normalize the time. */
	while (ts.tv_nsec >= 1000000000) {
		ts.tv_sec++;
		ts.tv_nsec -= 1000000000;
	}

	// spin timer
	while (client->state != SOCKST_CLOSED && pthread_cond_timedwait(&client->state_close_cond, &client->state_mutex, &ts) != ETIMEDOUT) ;

	state = client->state;
	pthread_mutex_unlock(&client->state_mutex);

	/* If already closed. */
	if (state == SOCKST_CLOSED)
		goto quit;

	// PERR("Timer expired, closing client %d\n", client->client_sock);

	close_client(client, 1);
quit:
	return (NULL);
}

int start_close_timeout(Client* client) {
	if (!ISVALIDSOCKET(client))
		return (-1);

	pthread_mutex_lock(&client->state_mutex);

	if (client->state != SOCKST_OPEN_WS)
		goto out;

	client->state = SOCKST_CLOSING;

	if (pthread_create(&client->thread_tout, NULL, close_timeout, client)) {
		pthread_mutex_unlock(&client->state_mutex);
		PERR(ESERVER, "Unable to create timeout thread\n");
	}
	client->close_thread = true;
out:
	pthread_mutex_unlock(&client->state_mutex);
	return 0;
}

int do_close(Frame *frame, int close_code) {
	int cc; /* Close code.           */

	/* If custom close-code. */
	if (close_code != -1) {
		cc = close_code;
		goto custom_close;
	}

	/* If empty or have a close reason, just re-send. */
	if (frame->size == 0 || frame->size > 2)
		goto send;

	/* Parse close code and check if valid, if not, we issue an protocol error.
	 */
	if (frame->size == 1)
		cc = frame->payload[0];
	else
		cc = ((int)frame->payload[0]) << 8 | frame->payload[1];

	/* Check if it's not valid, if so, we send a protocol error (1002). */
	if ((cc < 1000 || cc > 1003) && (cc < 1007 || cc > 1011) && (cc < 3000 || cc > 4999)) {
		cc = WS_CLSE_PROTERR;

	custom_close:
		frame->payload[0] = (cc >> 8);
		frame->payload[1] = (cc & 0xFF);

		if (ws_sendframe(frame->client, (const char *)frame->payload, sizeof(char) * 2, FRAME_CLOSE) < 0) {
			PERR(ESERVER, "An error has occurred while sending closing frame!\n");
			return -1;
		}
		BRED("CUSTOM CLOSE SEGF\n");
		return (0);
	}

	/* Send the data inside frame->payload. */
send:
	if (ws_sendframe(frame->client, (const char*)frame->payload, frame->size, FRAME_CLOSE) < 0) {
		PERR(ESERVER, "An error has occurred while sending closing frame!\n");
		return -1;
	}
	BRED("SEND SEGF\n");
	return (0);
}