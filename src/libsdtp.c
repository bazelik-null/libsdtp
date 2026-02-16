//
// Created by niko on 08.02.2026.
//

#include "libsdtp.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

// INSTANCE MANIPULATION //

sdtp_instance_t* sdtp_instance_create(const sdtp_config_t* config) {
	if (!config) return NULL;
	if (config->buffer_size == 0) return NULL;

	// Allocate an instance
	sdtp_instance_t* instance = (sdtp_instance_t*)malloc(sizeof(*instance));
	if (!instance) return NULL;

	// Set config pointer
	instance->config = *config;

	// Allocate buffers
	instance->input_buffer = sdtp_buffer_create(config, SDTP_INPUT_BUFFER);
	instance->output_buffer = sdtp_buffer_create(config, SDTP_OUTPUT_BUFFER);

	// If buffers allocation failed
	if (!instance->input_buffer || !instance->output_buffer) {
		sdtp_buffer_free(instance->input_buffer);
		sdtp_buffer_free(instance->output_buffer);
		free(instance);

		return NULL;
	}

	// Seed RNG
	srand(time(NULL));

	return instance;
}

void sdtp_instance_close(sdtp_instance_t* instance) {
	if (!instance) return;

	sdtp_buffer_free(instance->input_buffer);
	instance->input_buffer = NULL;
	sdtp_buffer_free(instance->output_buffer);
	instance->output_buffer = NULL;

	free(instance);
}


// PACKET MANIPULATION //

sdtp_packet_t* sdtp_construct_packet(const uint8_t* data, const uint32_t data_len, const sdtp_packet_type_t packet_type)
{
	if (!data && data_len > 0) return NULL;

	// Allocate packet struct
	sdtp_packet_t* packet = (sdtp_packet_t*)malloc(sizeof(sdtp_packet_t));
	if (!packet) return NULL;

	const uint32_t checksum = 0; // TODO: Calculate checksum

	// Header fields
	packet->header.id        = (uint32_t)rand();      // Random ID
	packet->header.data_size = data_len;              // Copy data len
	packet->header.type      = (uint32_t)packet_type; // Copy packet type
	packet->header.checksum  = checksum;              // Calculated data checksum

	if (data_len > 0) {
		// Allocate body
		packet->body = (uint8_t*)malloc(data_len);
		// If allocation failed
		if (!packet->body) {
			free(packet);
			return NULL;
		}

		// Copy data into a packet body
		memcpy(packet->body, data, data_len);
	} else {
		packet->body = NULL;
	}

	return packet;
}

void sdtp_packet_free(sdtp_packet_t* packet)
{
	if (!packet) return;

	if (packet->body) { free(packet->body); packet->body = NULL; }

	free(packet);
	packet = NULL;
}

uint8_t* sdtp_serialize_packet(const sdtp_packet_t* packet, size_t* out_size) {
    // Validate input pointers
    if (!packet || !out_size) return NULL;

    // Validate body pointer when data_size > 0
    if (packet->header.data_size > 0 && packet->body == NULL) return NULL;

    /*
     * Serialized layout:
     * Start of heading: 1 byte
     * Header: 4 * uint32_t (id, data_size, type, checksum)
     * Body: data_size bytes
     * Terminator: 1 byte
     */

    const size_t header_bytes = 4 * sizeof(uint32_t);
    const uint32_t data_size = packet->header.data_size;

    // Prevent integer overflow when computing total size:
    if (data_size > SIZE_MAX - (header_bytes + 1)) return NULL;

	// SoH + header + data + EoT
    const size_t packet_size = 1 + header_bytes + (size_t)data_size + 1;

    uint8_t* buffer = (uint8_t*)malloc(packet_size);
    if (!buffer) return NULL;

    uint8_t* write_ptr = buffer;
    size_t remaining = packet_size;

	// Write the SoH control character
	const uint8_t start_of_heading = SDTP_START_OF_HEADER;
	if (remaining < 1) { free(buffer); return NULL; }
	memcpy(write_ptr, &start_of_heading, 1);
	write_ptr += 1;
	remaining -= 1;

	// Get header data
	const uint32_t header_words[4] = {
		packet->header.id,
		packet->header.data_size,
		packet->header.type,
		packet->header.checksum
	};

	// Iterate each of the header elements and copy them
	for (size_t i = 0; i < 4; ++i) {
		uint32_t element = header_words[i];
		// Prevent buffer overflow
		if (remaining < sizeof(element)) { free(buffer); return NULL; }
		// Copy element
		memcpy(write_ptr, &element, sizeof(element));
		write_ptr += sizeof(element);
		remaining -= sizeof(element);
	}

	const uint32_t dsize = packet->header.data_size;

    // Copy body data
    if (dsize > 0) {
    	if (remaining < dsize) { free(buffer); return NULL; }
        memcpy(write_ptr, packet->body, dsize);
        write_ptr += dsize;
        remaining -= dsize;
    }

	// Write the EoT control character
	const uint8_t terminator = SDTP_TERMINATOR;
	if (remaining < 1) { free(buffer); return NULL; }
	memcpy(write_ptr, &terminator, 1);
	write_ptr += 1;
	remaining -= 1;

    // Check if all data was written
    if (remaining != 0) {
        free(buffer);
        return NULL;
    }

    *out_size = packet_size;
    return buffer;
}

sdtp_packet_t* sdtp_deserialize_packet(const uint8_t* buffer, const size_t buf_size) {
    if (!buffer) return NULL;

    const size_t header_bytes = 4 * sizeof(uint32_t);

    // Need at least SoH, header and terminator
    if (buf_size < 1 + header_bytes + 1) return NULL;

	const uint8_t* read_ptr = buffer;
	size_t remaining = buf_size;

    // Check SoH
    if (buffer[0] != SDTP_START_OF_HEADER) return NULL;
	read_ptr += 1;
	remaining -= 1;

    // Ensure header fits
    if (remaining < header_bytes + 1) return NULL; // header + at least terminator

    uint32_t header_words[4];
    for (size_t i = 0; i < 4; ++i) {
        if (remaining < sizeof(uint32_t)) return NULL;

        uint32_t element;

        memcpy(&element, read_ptr, sizeof(uint32_t));

        header_words[i] = element;

        read_ptr += sizeof(uint32_t);
        remaining -= sizeof(uint32_t);
    }

    const uint32_t id = header_words[0];
    const uint32_t data_size = header_words[1];
    const uint32_t type = header_words[2];
    const uint32_t checksum = header_words[3];

	// SoH + header + data + terminator
    const size_t packet_size = 1 + header_bytes + (size_t)data_size + 1;
    if (packet_size > buf_size) return NULL;

    // Ensure body fits in remaining buffer (remaining excludes SoH and header)
    if (remaining < (size_t)data_size + 1) return NULL; // body + terminator

    // Allocate packet struct for writing
    sdtp_packet_t* packet = (sdtp_packet_t*)malloc(sizeof(sdtp_packet_t));
    if (!packet) return NULL;

	// Copy header
    packet->header.id = id;
    packet->header.data_size = data_size;
    packet->header.type = type;
    packet->header.checksum = checksum;

	// Copy body
    if (data_size > 0) {
        packet->body = (uint8_t*)malloc(data_size);
        if (!packet->body) {
            free(packet);
            return NULL;
        }

        memcpy(packet->body, read_ptr, data_size);

        read_ptr += data_size;
        remaining -= data_size;
    } else {
        packet->body = NULL;
    }

    // Check terminator
    if (remaining < 1) {
        if (packet->body) free(packet->body);

        free(packet);
        return NULL;
    }
    if (*read_ptr != SDTP_TERMINATOR) {
        if (packet->body) free(packet->body);

        free(packet);
        return NULL;
    }

    return packet;
}

// BUFFER MANIPULATION //

sdtp_buffer_t* sdtp_buffer_create(const sdtp_config_t* config, const sdtp_buffer_type_t type) {
	// Allocate buffer struct
	sdtp_buffer_t* buffer = malloc(sizeof(sdtp_buffer_t));
	if (!buffer) {
		free(buffer);

		return NULL;
	}

	if (config->buffer_size > 0) {
		// Allocate data
		buffer->data = (uint8_t*)calloc(config->buffer_size, sizeof(uint8_t));
		if (!buffer->data) {
			free(buffer->data);
			free(buffer);

			return NULL;
		}
	} else {
		buffer->data = NULL;
	}

	// Set pointers
	buffer->head = buffer->data;
	buffer->tail = buffer->data;

	// Init variables
	buffer->size = config->buffer_size;
	buffer->type = type;

	return buffer;
}

void sdtp_buffer_free (sdtp_buffer_t* buffer) {
	if (!buffer) return;

	// Free data
	if (buffer->data) free(buffer->data);

	// Set pointers to null
	buffer->head = NULL;
	buffer->tail = NULL;
	buffer->data = NULL;

	// Free buffer struct
	free(buffer);
}

size_t sdtp_buffer_write(sdtp_instance_t* instance, const sdtp_buffer_type_t buffer_type, const uint8_t* source, const size_t write_len) {
	if (!instance || !source || write_len == 0) return 0;

	sdtp_buffer_t* buffer = sdtp_buffer_get_by_type(instance, buffer_type);
	if (!buffer) return 0;

	// If incoming data is larger than capacity, return error
	if (write_len >= buffer->size) {
		return 0;
	}

	const size_t used_space = sdtp_buffer_get_used_space(buffer);
	const size_t free_space = buffer->size - used_space;

	// If there's not enough free space, clear existing buffer contents
	if (write_len > free_space) {
		// Clear buffer
		memset(buffer->data, 0, used_space);

		// Write new data
		memcpy(buffer->data, source, write_len);
		buffer->tail = buffer->head + write_len;

		return write_len;
	}

	// If there's enough space append at end
	memcpy(buffer->tail, source, write_len);
	buffer->tail += write_len;

	return write_len;
}

size_t sdtp_buffer_read(sdtp_instance_t* instance, const sdtp_buffer_type_t buffer_type, uint8_t* destination, size_t read_len, const sdtp_read_mode_t mode) {
	if (!instance || !destination || read_len == 0) return 0;

	sdtp_buffer_t* buffer = sdtp_buffer_get_by_type(instance, buffer_type);
	if (!buffer) return 0;

	const size_t used_space = sdtp_buffer_get_used_space(buffer);

	// Length of the valid data
	if (used_space == 0) return 0;
	if (read_len > used_space) read_len = used_space;

	// Copy data
	memcpy(destination, buffer->data, read_len);

	// Handle different read modes
	switch (mode) {
		case SDTP_READ_FULL:
			memset(buffer->data, 0, buffer->size);
			buffer->tail = buffer->data;
			break;

		case SDTP_READ_PARTIAL:
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
	buffer->tail = buffer->head;
}

size_t sdtp_buffer_get_used_space(const sdtp_buffer_t* buffer) {
	return buffer->tail - buffer->head;
}

sdtp_buffer_t* sdtp_buffer_get_by_type(sdtp_instance_t* instance, const sdtp_buffer_type_t buffer_type) {
	if (!instance) return NULL;

	if (buffer_type == SDTP_OUTPUT_BUFFER) {
		return instance->output_buffer;
	}
	if (buffer_type == SDTP_INPUT_BUFFER) {
		return instance->input_buffer;
	}

	return NULL;
}

// I/O MANIPULATION //

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
