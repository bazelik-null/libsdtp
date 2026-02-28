//
// Created by niko on 23.02.2026.
//

#include <api/libsdtp.h>

#include <stdlib.h>
#include <string.h>

char* sdtp_get_char_data(const sdtp_packet_t* packet) {
	if (packet == NULL) return NULL;

	const uint8_t* body = packet->body;
	if (body == NULL) return NULL;

	// Allocate a buffer for a null-terminated string
	char* body_buffer = malloc(packet->header.data_size + 1);
	if (body_buffer == NULL) return NULL;

	// Copy the bytes to a buffer
	memcpy(body_buffer, body, packet->header.data_size);

	// Null-terminate the string
	body_buffer[packet->header.data_size] = '\0';

	return body_buffer;
}

uint8_t* sdtp_char_to_bytes(const char* source, size_t* data_len) {
	if (source == NULL || data_len == NULL) return NULL;

	*data_len = strlen(source);

	if (*data_len == 0) {
		return NULL;
	}

	uint8_t* byte_buffer = malloc(*data_len);
	if (byte_buffer == NULL) return NULL;

	// Copy the bytes to a buffer
	memcpy(byte_buffer, (const uint8_t*)source, *data_len);

	return byte_buffer;
}