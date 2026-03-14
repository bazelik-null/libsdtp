// Copyright (c) 2026 bazelik-null

#ifndef LIBSDTP_SHARED_H
#define LIBSDTP_SHARED_H

#include <stdint.h>

/**
 * @brief Struct with pointers to functions.
 * Used to read/write from/to buffers.
 **/
typedef struct {
	void     (*write)(uint8_t* buffer, size_t write_len); // Writes a buffer to the output channel.
	uint8_t* (*read)(size_t* read_len);                   // Returns a buffer from the input channel. Writes read length to read_len.
} sdtp_function_hooks;

#endif //LIBSDTP_SHARED_H