// Copyright (c) 2026 bazelik

#include <api/libsdtp.h>

#include <stdlib.h>
#include <string.h>

sdtp_buffer_t* sdtp_buffer_create(const sdtp_config_t* config) {
	if (config->buffer_size <= 0) return NULL;

	// Allocate buffer struct
	sdtp_buffer_t* buffer = malloc(sizeof(sdtp_buffer_t));
	if (!buffer) {
		return NULL;
	}

	// Allocate data
	buffer->data = (uint8_t*)calloc(config->buffer_size, sizeof(uint8_t));
	if (!buffer->data) {
		free(buffer);
		return NULL;
	}

	buffer->tail = buffer->data;

	// Init variables
	buffer->size = config->buffer_size;

	return buffer;
}

void sdtp_buffer_free(sdtp_buffer_t* buffer) {
	if (!buffer) return;

	// Free data
	if (buffer->data) free(buffer->data);

	// Set pointers to null
	buffer->tail = NULL;
	buffer->data = NULL;

	// Free buffer struct
	free(buffer);
}

size_t sdtp_buffer_write(sdtp_buffer_t* buffer, const uint8_t* source, const size_t write_len) {
	if (!buffer || !source || write_len == 0) return 0;

	const size_t used_space = sdtp_buffer_get_used_space(buffer);
	const size_t free_space = buffer->size - used_space;

	// If there's not enough free space, clear existing buffer contents
	if (write_len > free_space) {
		// Clear buffer
		memset(buffer->data, 0, used_space);

		// Write new data
		memcpy(buffer->data, source, write_len);
		buffer->tail = buffer->data + write_len;

		return write_len;
	}

	// If there's enough space append at end
	memcpy(buffer->tail, source, write_len);
	buffer->tail += write_len;

	return write_len;
}

size_t sdtp_buffer_read(sdtp_buffer_t* buffer, uint8_t* destination, size_t read_len, const sdtp_read_mode_t mode) {
	if (!buffer || !destination || read_len == 0) return 0;

	const size_t used_space = sdtp_buffer_get_used_space(buffer);

	// Length of the valid data
	if (used_space == 0) return 0;
	if (read_len > used_space) read_len = used_space;

	// Copy data
	memcpy(destination, buffer->data, read_len);

	// Handle different read modes
	switch (mode) {
		case SDTP_READ_FULL:
			// Clear buffer
			memset(buffer->data, 0, buffer->size);
			buffer->tail = buffer->data;
			break;

		case SDTP_READ_PARTIAL:
			// Shift data
			if (read_len < used_space) {
				memmove(buffer->data, buffer->data + read_len, used_space - read_len);
			}
			buffer->tail -= read_len;
			break;

		case SDTP_READ_PEEK:
			// Do nothing, just return data
			break;
	}

	return read_len;
}

void sdtp_buffer_clear(sdtp_instance_t* instance, const sdtp_buffer_type_t buffer_type) {
	if (!instance) return;

	sdtp_buffer_t* buffer = sdtp_buffer_get_by_type(instance, buffer_type);
	if (!buffer) return;

	const size_t used = sdtp_buffer_get_used_space(buffer);
	if (used == 0) return;

	// Remove used space
	memset(buffer->data, 0, used);
	buffer->tail = buffer->data;
}

size_t sdtp_buffer_get_used_space(const sdtp_buffer_t* buffer) {
	if (!buffer || !buffer->data) return 0;
	return buffer->tail - buffer->data;
}

sdtp_buffer_t* sdtp_buffer_get_by_type(sdtp_instance_t* instance, const sdtp_buffer_type_t buffer_type) {
	if (!instance) return NULL;

	if (buffer_type == SDTP_OUTPUT_BUFFER && instance->output_buffer) {
		return instance->output_buffer;
	}
	if (buffer_type == SDTP_INPUT_BUFFER && instance->input_buffer) {
		return instance->input_buffer;
	}

	return NULL;
}