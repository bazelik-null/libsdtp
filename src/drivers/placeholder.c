// Copyright (c) 2026 bazelik-null

#include <stdlib.h>
#include <drivers/libsdtp_hal.h>

uint32_t sdtp_hal_rand(void) {
	return arc4random();
}
