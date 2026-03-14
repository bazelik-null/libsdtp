// Copyright (c) 2026 bazelik

#include <api/libsdtp.h>

#include <stdlib.h>
#include <time.h>

sdtp_instance_t* sdtp_instance_create(const sdtp_config_t* config, const sdtp_function_hooks* gpio_hooks) {
	if (!config) return NULL;
	if (config->buffer_size == 0) return NULL;
	if (!gpio_hooks) return NULL;

	// Allocate an instance
	sdtp_instance_t* instance = (sdtp_instance_t*)malloc(sizeof(*instance));
	if (!instance) return NULL;

	// Set config
	instance->config = *config;

	// Set hooks
	instance->function_hooks = gpio_hooks;

	// Allocate buffers
	instance->input_buffer = sdtp_buffer_create(config);
	instance->output_buffer = sdtp_buffer_create(config);

	// If buffers allocation failed
	if (!instance->input_buffer || !instance->output_buffer) {
		sdtp_buffer_free(instance->input_buffer);
		sdtp_buffer_free(instance->output_buffer);
		free(instance);

		return NULL;
	}

	return instance;
}

void sdtp_instance_close(sdtp_instance_t* instance) {
	if (!instance) return;

	sdtp_buffer_free(instance->input_buffer);
	instance->input_buffer = NULL;
	sdtp_buffer_free(instance->output_buffer);
	instance->output_buffer = NULL;

	free(instance);
	instance = NULL;
}
