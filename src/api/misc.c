//
// Created by niko on 23.02.2026.
//

#include <api/libsdtp.h>

#include <stdlib.h>
#include <string.h>

char* sdtp_get_char_data(const sdtp_packet_t* packet) {
	if (packet == NULL) return NULL;

	const uint8_t* body = packet->body;

	// Allocate a buffer for a null-terminated string
	char* body_buffer = malloc(packet->header.data_size + 1);

	// Copy the bytes to a buffer
	for (int i = 0; i < packet->header.data_size; i++) {
		body_buffer[i] = (char)body[i];
	}

	// Null-terminate the string
	body_buffer[packet->header.data_size] = '\0';

	return body_buffer;
}

uint8_t* sdtp_char_to_bytes(const char* source, size_t* data_len) {
	if (source == NULL || data_len == NULL) return NULL;

	*data_len = strlen(source);

	// Allocate a buffer for the uint8_t array
	uint8_t* byte_buffer = malloc(*data_len);

	if (byte_buffer == NULL) return NULL;

	// Copy the characters to the byte buffer
	for (size_t i = 0; i < *data_len; i++) {
		byte_buffer[i] = (uint8_t)source[i];
	}

	return byte_buffer;
}