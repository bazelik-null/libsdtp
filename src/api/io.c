// Copyright (c) 2026 bazelik-null

#include <stdlib.h>
#include <api/libsdtp.h>

bool sdtp_io_write(sdtp_instance_t* instance) {
	if (!instance || !instance->function_hooks->write) return false;

	// Get used space of the input buffer
	const size_t used_space = sdtp_buffer_get_used_space(instance->output_buffer);

	// Allocate tmp buffer
	uint8_t* tmp_buffer = (uint8_t*)malloc(used_space);
	if (!tmp_buffer) return false;

	// Read buffer data
	const size_t read = sdtp_buffer_read(instance->output_buffer, tmp_buffer, used_space, SDTP_READ_PARTIAL);
	if (read == 0) {
		free(tmp_buffer);
		return false;
	}

	// Write tmp buffer via function hook
	instance->function_hooks->write(tmp_buffer, read);

	// Clean memory
	free(tmp_buffer);

	return true;
}

bool sdtp_io_read(sdtp_instance_t* instance) {
	if (!instance || !instance->function_hooks->read) return false;

	// Read data to tmp buffer via function hook
	size_t read_len = 0;
	uint8_t* tmp_buffer = instance->function_hooks->read(&read_len);
	if (read_len == 0 || !tmp_buffer) {
		if (tmp_buffer) free(tmp_buffer);
		return false;
	}

	// Write data from tmp buffer to input buffer
	const size_t written = sdtp_buffer_write(instance->input_buffer, tmp_buffer, read_len);
	if (written != read_len) {
		free(tmp_buffer);
		return false;
	}

	// Clean memory
	free(tmp_buffer);

	return true;
}