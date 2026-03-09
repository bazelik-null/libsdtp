// Copyright (c) 2026 bazelik

#include <api/libsdtp.h>

#include <stdlib.h>

bool sdtp_write_packet(sdtp_instance_t* instance, const sdtp_packet_t* packet) {
	if (!instance || !packet) return false;

	// Get buffer
	sdtp_buffer_t* buffer = sdtp_buffer_get_by_type(instance, SDTP_OUTPUT_BUFFER);
	if (!buffer) {
		return false;
	}

	// Serialize packet into a temporary buffer
	size_t serialized_size = 0;
	uint8_t* serialized = sdtp_serialize_packet(packet, &serialized_size);
	if (!serialized) return false;

	// Ensure serialized packet fits into buffer
	if (serialized_size > instance->config.buffer_size) {
		free(serialized);
		return false;
	}

	// Attempt to write serialized packet into output buffer
	const size_t written = sdtp_buffer_write(buffer, serialized, serialized_size);
	free(serialized);

	// If not all bytes were written
	if (written != serialized_size) return false;

	// TODO: Call HAL and send data.

	return true;
}

sdtp_packet_t* sdtp_read_packet(sdtp_instance_t* instance, sdtp_read_mode_t mode) {
	if (!instance) return NULL;

	// Get buffer
	sdtp_buffer_t* buffer = sdtp_buffer_get_by_type(instance, SDTP_INPUT_BUFFER);
	if (!buffer) {
		return NULL;
	}

	// TODO: Get only last packet
	const size_t read_len = sdtp_buffer_get_used_space(instance->input_buffer);

	// Allocate serialized buffer and copy packet bytes
	uint8_t* serialized = (uint8_t*)malloc(read_len);
	if (!serialized) return NULL;

	const size_t serialized_len = sdtp_buffer_read(buffer, serialized, read_len, mode);

	// Deserialize
	sdtp_packet_t* packet = sdtp_deserialize_packet(serialized, serialized_len);
	free(serialized);

	return packet;
}