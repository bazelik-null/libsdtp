//
// Created by niko on 23.02.2026.
//

#include <include/api/libsdtp.h>

#include <stdlib.h>

sdtp_error_t sdtp_write_packet(sdtp_instance_t* instance, const sdtp_packet_t* packet) {
	if (!instance || !packet) return SDTP_UNDEFINED;

	// Serialize packet into a temporary buffer
	size_t serialized_size = 0;
	uint8_t* serialized = sdtp_serialize_packet(packet, &serialized_size);
	if (!serialized) return SDTP_INVALID_PACKET;

	// Ensure serialized packet fits into buffer
	if (serialized_size > instance->config.buffer_size) {
		free(serialized);
		return SDTP_BUFFER_FAIL;
	}

	// Attempt to write whole serialized packet into output buffer
	const size_t written = sdtp_buffer_write(instance, SDTP_OUTPUT_BUFFER, serialized, serialized_size);
	free(serialized);

	// If not all bytes were written
	if (written != serialized_size) return SDTP_BUFFER_FAIL;

	return SDTP_OK;
}

sdtp_packet_t* sdtp_read_packet(sdtp_instance_t* instance) {
	if (!instance) return NULL;

	const size_t read_len = sdtp_buffer_get_used_space(instance->input_buffer);

	// Allocate serialized buffer and copy packet bytes
	uint8_t* serialized = (uint8_t*)malloc(read_len);
	if (!serialized) return NULL;
	const size_t serialized_len = sdtp_buffer_read(instance, SDTP_INPUT_BUFFER, serialized, read_len, SDTP_READ_PARTIAL);

	// Deserialize
	sdtp_packet_t* packet = sdtp_deserialize_packet(serialized, serialized_len);
	free(serialized);

	return packet;
}