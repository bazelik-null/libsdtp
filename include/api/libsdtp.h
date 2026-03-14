// Copyright (c) 2026 bazelik

#ifndef SDTP_H
#define SDTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <hooks/libsdtp_hooks.h>

#define SDTP_TERMINATOR (uint8_t)0x04
#define SDTP_START_OF_HEADER (uint8_t)0x02

// INSTANCE AND CONFIG //

/**
 * Buffer types.
 * Can be either input or output buffer.
 **/
typedef enum {
	SDTP_INPUT_BUFFER = 0,
	SDTP_OUTPUT_BUFFER = 1,
} sdtp_buffer_type_t;

/**
 * LILO linear buffer for storing serial data.
 * @param size Total buffer size.
 * @param tail Current position.
 * @param data Pointer to start of the buffer memory.
 **/
typedef struct {
	size_t size;

	uint8_t* tail;

	uint8_t* data;
} sdtp_buffer_t;

/**
 * Reading mode for buffer.
 * @param SDTP_READ_FULL Read and clear entire buffer
 * @param SDTP_READ_PARTIAL Read and clear used data
 * @param SDTP_READ_PEEK Read without modifying buffer
 **/
typedef enum {
	SDTP_READ_FULL,      // Read and clear entire buffer
	SDTP_READ_PARTIAL,   // Read and clear used data
	SDTP_READ_PEEK       // Read without modifying buffer
} sdtp_read_mode_t;

/**
 * SDTP configuration.
 * @param input_bus_pin Port number for input channel.
 * @param output_bus_pin Port number for output channel.
 * @param buffer_size Buffers size.
 * @param baud_rate Baud rate (bits per second).
 **/
typedef struct {
	uint8_t input_bus_pin;
	uint8_t output_bus_pin;

	size_t buffer_size;

	uint32_t baud_rate;
} sdtp_config_t;

/**
 * Single SDTP instance.
 * Contains I/O buffers and config.
 * Should be created only with sdtp_instance_create().
 **/
typedef struct {
	sdtp_config_t config;

	sdtp_buffer_t* input_buffer;
	sdtp_buffer_t* output_buffer;

	const sdtp_function_hooks* function_hooks;
} sdtp_instance_t;

// PACKETS //

/**
 * Packet types.
 **/
typedef enum {
	SDTP_HANDSHAKE   = 0,
	SDTP_DISCONNECT  = 1,
	SDTP_ERROR       = 2,
	SDTP_DATA_PACKET = 3,
} sdtp_packet_type_t;

/**
 * Header section of the packet.
 * Contains:
 * @param id Packet ID
 * @param data_size Body block size in bytes
 * @param type Packet type (enum sdtp_packet_type_t)
 * @param checksum Checksum
 **/
typedef struct {
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
typedef struct {
	sdtp_packet_header_t header; // Packet header
	uint8_t* body;               // Packet body
} sdtp_packet_t;

/*******************************************************
 * Serialized packet layout:
 * Start of heading: 1 byte
 * Header: 4 * uint32_t (id, data_size, type, checksum)
 * Body: data_size bytes
 * Terminator: 1 byte
 ******************************************************/

// INSTANCE MANIPULATION //

/**
 * @brief Creates new SDTP instance and returns pointer to it.
 * Config is copied.
 **/
sdtp_instance_t* sdtp_instance_create(const sdtp_config_t* config, const sdtp_function_hooks* gpio_hooks);
/**
 * @brief Frees and closes instance.
 * Config is not freed.
 **/
void sdtp_instance_close(sdtp_instance_t* instance);

// PACKET MANIPULATION //

/**
 * @brief Allocates and constructs a new packet.
 * Caller must free returned pointer.
 * @param data Buffer with packet data.
 * @param packet_type Type of the packet (enum sdtp_packet_type_t)
 * @param packet_id Packet ID (must be random).
 * @return Pointer to allocated packet struct.
 **/
sdtp_packet_t* sdtp_construct_packet(const char* data, sdtp_packet_type_t packet_type, uint32_t packet_id);
/**
 * @brief Frees packet and body data.
 **/
void sdtp_packet_free(sdtp_packet_t* packet);

/**
 * @brief Serializes packet to a newly allocated buffer.
 * Returned data is in host byte order.
 * Caller must free returned pointer.
 * @param packet Pointer to target packet.
 * @param out_size Var which receives the size in bytes of the returned buffer.
 * @return Pointer to allocated buffer with serialized packet.
 **/
uint8_t* sdtp_serialize_packet(const sdtp_packet_t* packet, size_t* out_size);
/**
 * @brief Deserializes raw byte data from a buffer to a newly allocated packet.
 * Returned data is in sender host byte order.
 * Caller must free returned pointer.
 * @param buffer Buffer with serialized target packet.
 * @param buf_size Size of the buffer.
 * @return Pointer to allocated packet struct.
 **/
sdtp_packet_t* sdtp_deserialize_packet(const uint8_t* buffer, size_t buf_size);

// BUFFER MANIPULATION //

/**
 * @brief Creates a new buffer.
 * Caller must free returned pointer.
 * @param config Pointer to SDTP configuration.
 * @return Pointer to allocated data buffer.
 **/
sdtp_buffer_t* sdtp_buffer_create(const sdtp_config_t* config);
/**
 * @brief Frees buffer and data.
 * Caller must set buffer pointer to NULL.
 **/
void sdtp_buffer_free(sdtp_buffer_t* buffer);

/**
 * @brief Writes byte stream into the buffer.
 * @param buffer Buffer to write.
 * @param source Buffer with data to write.
 * @param write_len Buffer length.
 * @return Written length.
 **/
size_t sdtp_buffer_write(sdtp_buffer_t* buffer, const uint8_t* source, size_t write_len);
/**
 * @brief Reads byte stream from the buffer.
 * @param buffer Buffer to read.
 * @param destination Buffer into which data will be copied.
 * @param read_len Length to read.
 * @param mode Reading mode (enum sdtp_read_mode_t).
 * @return Read length.
 **/
size_t sdtp_buffer_read(sdtp_buffer_t* buffer, uint8_t* destination, size_t read_len, sdtp_read_mode_t mode);

/**
 * @brief Clears buffer.
 * @param instance SDTP instance.
 * @param buffer_type Type of buffer to clear.
 **/
void sdtp_buffer_clear(sdtp_instance_t* instance, sdtp_buffer_type_t buffer_type);

/**
 * @brief Gets used buffer space.
 **/
size_t sdtp_buffer_get_used_space(const sdtp_buffer_t* buffer);
/**
 * @brief Gets pointer to a buffer from an SDTP instance by type.
 **/
sdtp_buffer_t* sdtp_buffer_get_by_type(sdtp_instance_t* instance, sdtp_buffer_type_t buffer_type);

// INSTANCE BUFFER MANIPULATION //

/**
 * @brief Writes a single packet into output buffer.
 * Packet is not freed.
 * @param instance SDTP instance.
 * @param packet Packet to write.
 * @return Status (false - error, true - success).
 **/
bool sdtp_write_packet(sdtp_instance_t* instance, const sdtp_packet_t* packet);
/**
 * @brief Reads a single packet from the input buffer and returns pointer to it.
 * Caller must free returned pointer.
 * @param instance SDTP instance.
 * @param mode Reading mode (enum sdtp_read_mode_t).
 * @return Pointer to allocated packet struct.
 **/
sdtp_packet_t* sdtp_read_packet(sdtp_instance_t* instance, sdtp_read_mode_t mode);

// IO MANIPULATION //

/**
 * @brief Writes data from output buffer to IO output via function hook.
 * @return Status (false - error, true - success).
 **/
bool sdtp_io_write(sdtp_instance_t* instance);
/**
 * @brief Writes data from IO input to input buffer via function hook.
 * @return Status (false - error, true - success).
 */
bool sdtp_io_read(sdtp_instance_t* instance);

// MISC //

/**
 * @brief Converts packet data to char.
 * Caller must free returned and packet pointer.
 * @return packet->body converted from raw bytes to null-terminated char.
 */
char* sdtp_get_char_data(const sdtp_packet_t* packet);
/**
 * @brief Converts chars to uint8_t.
 * Caller must free returned and source buffer pointer.
 * @param source Buffer with data to convert.
 * @param data_len Converted bytes length.
 * @return Buffer with converted data.
 */
uint8_t* sdtp_char_to_bytes(const char* source, size_t* data_len);

/**
 * @brief Calculates Fletcher-32 checksum
 */
uint32_t sdtp_calculate_fletcher32(const uint8_t *data, size_t len);
/**
 * @brief Verifies Fletcher-32 checksum
 */
bool sdtp_verify_fletcher32(const uint8_t *data, size_t length, uint32_t checksum);

#ifdef __cplusplus
}
#endif

#endif //SDTP_H
