/**
 * Created by niko on 08.02.2026.
 **/

#ifndef SDTP_H
#define SDTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define SDTP_TERMINATOR (uint8_t)0x04
#define SDTP_START_OF_HEADER (uint8_t)0x02

// INSTANCE AND CONFIG //

/**
 * TODO: Implement more device types.
 **/
typedef enum sdtp_device_type_t {
	SDTP_CONTROLLER = 0
} sdtp_device_type_t;

/**
 * Buffer types.
 * Can be either input or output buffer.
 **/
typedef enum sdtp_buffer_type_t {
	SDTP_INPUT_BUFFER = 0,
	SDTP_OUTPUT_BUFFER = 1,
} sdtp_buffer_type_t;

/**
 * Linear buffer. 
 * Type: Last In Last Out.
 * Contains:
 * 1. Buffer type (either input or output).
 * 2. Total buffer size.
 * 3. Current position.
 * 4. Start of used data.
 * 5. Pointer to the buffer memory.
 **/
typedef struct sdtp_buffer_t {
	sdtp_buffer_type_t type;

	size_t size;

	uint8_t* tail;
	uint8_t* head;

	uint8_t* data;
} sdtp_buffer_t;

/**
 * Reading mode for buffer.
 **/
typedef enum sdtp_read_mode_t {
	SDTP_READ_FULL,      // Read and clear entire buffer
	SDTP_READ_PARTIAL,   // Read and clear used data
	SDTP_READ_PEEK       // Read without modifying buffer
} sdtp_read_mode_t;

/**
 * SDTP configuration. 
 * Contains:
 * 1. Port numbers for I/O channels.
 * 2. Buffers size.
 * 3. Baud rate (bits per second).
 * 4. Device ID.
 * 5. Type of the device (enum sdtp_device_type_t).
 **/
typedef struct sdtp_config_t {
	uint8_t input_bus_pin;
	uint8_t output_bus_pin;

	size_t buffer_size;

	uint32_t baud_rate;

	uint32_t device_id;
	sdtp_device_type_t device_type;
} sdtp_config_t;

/**
 * Single SDTP instance. 
 * Contains I/O buffers and config. 
 * Should be created only with sdtp_instance_create().
 **/
typedef struct sdtp_instance_t {
	sdtp_config_t config;

	sdtp_buffer_t* input_buffer;
	sdtp_buffer_t* output_buffer;
} sdtp_instance_t;

// ERRORS //

/**
 * Error types.
 * Errors are negative. OK = 0.
 **/
typedef enum sdtp_error_t {
	SDTP_INVALID_CONNECTION = -4,
	SDTP_INVALID_PACKET     = -3,
	SDTP_BUFFER_FAIL        = -2,
	SDTP_UNDEFINED          = -1,
	SDTP_OK                 = 0
} sdtp_error_t;

// PACKETS //

/**
 * Packet types.
 **/
typedef enum sdtp_packet_type_t {
	SDTP_HANDSHAKE   = 0,
	SDTP_DISCONNECT  = 1,
	SDTP_ERROR       = 2,
	SDTP_DATA_PACKET = 3,
	SDTP_DATA_STREAM = 4
} sdtp_packet_type_t;

/**
 * Header section of the packet. 
 * Contains:
 * 1. Packet ID
 * 2. Body block size in bytes
 * 3. Packet type (enum sdtp_packet_type_t)
 * 4. Checksum
 **/
typedef struct sdtp_packet_header_t {
	uint32_t id;
	uint32_t data_size;
	uint32_t type;
	uint32_t checksum;
} sdtp_packet_header_t;

/**
 * Standard SDTP packet. 
 * Contains packet header and body. 
 * Should be created only with sdtp_construct_packet().
 **/
typedef struct sdtp_packet_t {
	sdtp_packet_header_t header; // Packet header
	uint8_t* body;               // Packet body
} sdtp_packet_t;

// INSTANCE MANIPULATION //

/**
 * Creates new SDTP instance and returns pointer to it. 
 * Config is copied.
 **/
sdtp_instance_t* sdtp_instance_create(const sdtp_config_t* config);
/**
 * Frees and closes instance. 
 * Caller must set instance pointer to NULL. 
 * Config is not freed.
 **/
void sdtp_instance_close(sdtp_instance_t* instance);

// PACKET MANIPULATION //

/**
 * Allocates and constructs a new packet. 
 * Caller must free returned pointer.
 **/
sdtp_packet_t* sdtp_construct_packet(const uint8_t* data, uint32_t data_len, sdtp_packet_type_t packet_type);
/**
 * Frees packet and body data.
 **/
void sdtp_packet_free(sdtp_packet_t* packet);

/**
 * Serializes packet to a newly allocated buffer. 
 * out_size receives the size in bytes of the returned buffer. 
 * Returned data is in host byte order. 
 * Caller must free returned pointer.
 **/
uint8_t* sdtp_serialize_packet(const sdtp_packet_t* packet, size_t* out_size);
/**
 * Deserializes raw byte data from a buffer to a newly allocated packet. 
 * Returned data is in sender host byte order. 
 * Caller must free returned pointer.
 **/
sdtp_packet_t* sdtp_deserialize_packet(const uint8_t* buffer, size_t buf_size);

// BUFFER MANIPULATION //

/**
 * Creates a new buffer. 
 * Caller must free returned pointer.
 **/
sdtp_buffer_t* sdtp_buffer_create(const sdtp_config_t* config, sdtp_buffer_type_t type);
/**
 * Frees buffer and data. 
 * Caller must set buffer pointer to NULL. 
 **/
void sdtp_buffer_free(sdtp_buffer_t* buffer);

/**
 * Writes byte stream into the buffer and returns written length.
 **/
size_t sdtp_buffer_write(sdtp_instance_t* instance, sdtp_buffer_type_t buffer_type, const uint8_t* source, size_t write_len);
/**
 * Reads byte stream from the buffer and returns read length.
 **/
size_t sdtp_buffer_read(sdtp_instance_t* instance, sdtp_buffer_type_t buffer_type, uint8_t* destination, size_t read_len, sdtp_read_mode_t mode);

/**
 * Clears buffer.
 **/
void sdtp_buffer_clear(sdtp_instance_t* instance, sdtp_buffer_type_t buffer_type);

/**
 * Gets used buffer space.
 **/
size_t sdtp_buffer_get_used_space(const sdtp_buffer_t* buffer);
/**
 * Gets buffer instance pointer by type.
 **/
sdtp_buffer_t* sdtp_buffer_get_by_type(sdtp_instance_t* instance, sdtp_buffer_type_t buffer_type);

// I/O MANIPULATION //

/**
 * Writes a single packet into output buffer.
 **/
sdtp_error_t sdtp_write_packet(sdtp_instance_t* instance, const sdtp_packet_t* packet);
/**
 * Reads a single packet from the input buffer and returns pointer to it. 
 * Doesn't advance buffers after reading 
 * Caller must free returned pointer.
 **/
sdtp_packet_t* sdtp_read_packet(sdtp_instance_t* instance);

#ifdef __cplusplus
}
#endif

#endif //SDTP_H
