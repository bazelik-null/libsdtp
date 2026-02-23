//
// Created by niko on 23.02.2026.
//

// libsdtp Hardware Abstraction Layer

#ifndef SDTP_HAL_H
#define SDTP_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// TODO: Implement all of this
// !!! WORK IN PROGRESS !!!

/**
 * Structure with array of bitwise commands for data transmission.
 * One element (uint8_t) holds 8 commands (bits)
 **/
typedef struct sdtp_hal_data_stream_t {
	uint8_t *bit_array;

	size_t size;
} sdtp_hal_bit_stream_t;

/**
 * Converts byte data to bit stream.
 * Caller must free returned pointer.
 * @param source Buffer with data to convert.
 * @param size Source buffer size.
 * @return Bit stream
 **/
sdtp_hal_bit_stream_t* sdtp_hal_convert_to_bit_stream(const uint8_t* source, size_t size);
/**
 * Converts bit stream to byte data.
 * Caller must free returned pointer.
 * @param source Struct with data to convert.
 * @return Buffer with converted data.
 **/
uint8_t* sdtp_hal_convert_from_bit_stream(const sdtp_hal_bit_stream_t* source);

/**
 * Sends data to specified channel.
 * @param source Bit stream to send.
 * @param channel Channel (pin) to write.
 **/
void sdtp_hal_write_bytes(const sdtp_hal_bit_stream_t* source, uint8_t channel);
/**
 * Reads data from specified channel.
 * Caller must free returned pointer.
 * @param channel Channel (pin) to read.
 * @return Read bit stream.
 **/
sdtp_hal_bit_stream_t* sdtp_hal_read_bytes(uint8_t channel);

/**
 * Generates a random value.
 * @return Value.
 */
int sdtp_hal_rand();

#ifdef __cplusplus
}
#endif

#endif //SDTP_HAL_H