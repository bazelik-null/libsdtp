//
// Created by niko on 23.02.2026.
//

#include <api/libsdtp.h>

#include <stdlib.h>

char* sdtp_get_char_data(const sdtp_packet_t* packet) {
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