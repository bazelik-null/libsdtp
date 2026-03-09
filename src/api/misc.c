// Copyright (c) 2026 bazelik

#include <api/libsdtp.h>

#include <stdlib.h>
#include <string.h>

char* sdtp_get_char_data(const sdtp_packet_t* packet) {
	if (packet == NULL) return NULL;

	if (packet->header.data_size == 0) return NULL;

	const uint8_t* body = packet->body;
	if (body == NULL) return NULL;

	// Allocate a buffer for a null-terminated string
	char* body_buffer = malloc(packet->header.data_size + 1);
	if (body_buffer == NULL) return NULL;

	// Check if body size is equal to data_size
	if (sizeof(body) != packet->header.data_size + 1) {
		free(body_buffer);
		return NULL;
	}

	// Copy the bytes to a buffer
	memcpy(body_buffer, (const char*)body, packet->header.data_size);

	// Null-terminate the string
	body_buffer[packet->header.data_size] = '\0';

	return body_buffer;
}

uint8_t* sdtp_char_to_bytes(const char* source, size_t* data_len) {
	if (source == NULL || data_len == NULL) return NULL;

	*data_len = sizeof(source);

	if (*data_len == 0) {
		return NULL;
	}

	uint8_t* byte_buffer = malloc(*data_len);
	if (byte_buffer == NULL) return NULL;

	// Copy the bytes to a buffer
	memcpy(byte_buffer, (const uint8_t*)source, *data_len);

	return byte_buffer;
}

// Adapted from Wikipedia's Fletcher checksum article
// https://en.wikipedia.org/wiki/Fletcher%27s_checksum
// Original content licensed under CC BY-SA
uint32_t sdtp_calculate_fletcher32(const uint8_t *data, const size_t len)
{
	uint32_t c0, c1;
	uint i;

	size_t word_len = len / 2;
	const uint16_t *data16 = (const uint16_t *)data;

	for (c0 = c1 = 0; word_len >= 360; word_len -= 360) {
		for (i = 0; i < 360; ++i) {
			c0 = c0 + *data16++;
			c1 = c1 + c0;
		}
		c0 = c0 % 65535;
		c1 = c1 % 65535;
	}

	for (i = 0; i < word_len; ++i) {
		c0 = c0 + *data16++;
		c1 = c1 + c0;
	}

	if (len & 1) {
		c0 = c0 + ((uint8_t *)data16)[0];
		c1 = c1 + c0;
	}

	c0 = c0 % 65535;
	c1 = c1 % 65535;

	return (c1 << 16 | c0);
}

bool sdtp_verify_fletcher32(const uint8_t *data, const size_t length, const uint32_t checksum) {
	return sdtp_calculate_fletcher32(data, length) == checksum;
}
