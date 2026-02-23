//
// Created by niko on 23.02.2026.
//

#include <api/libsdtp.h>

#include <stdlib.h>
#include <string.h>

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