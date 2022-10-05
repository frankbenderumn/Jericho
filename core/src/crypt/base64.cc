#include "crypt/utf8.h"

static
uint32_t decode(uint32_t* state, uint32_t* codep, uint32_t byte) {
	uint32_t type = utf8d[byte];

	*codep = (*state != UTF8_ACCEPT) ?
		(byte & 0x3fu) | (*codep << 6) :
		(0xff >> type) & (byte);

	*state = utf8d[256 + *state*16 + type];
	return *state;
}

int is_utf8(uint8_t *s) {
	uint32_t codepoint, state = 0;

	while (*s)
		decode(&state, &codepoint, *s++);

	return state == UTF8_ACCEPT;
}

int is_utf8_len(uint8_t *s, size_t len) {
	uint32_t codepoint, state = 0;
	size_t i;

	for (i = 0; i < len; i++)
		decode(&state, &codepoint, *s++);

	return state == UTF8_ACCEPT;
}

uint32_t is_utf8_len_state(uint8_t *s, size_t len, uint32_t state) {
	uint32_t codepoint;
	size_t i;

	for (i = 0; i < len; i++)
		decode(&state, &codepoint, *s++);

	return state;
}