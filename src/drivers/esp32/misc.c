//
// Created by niko on 23.02.2026.
//

#include <stdlib.h>
#include <drivers/libsdtp_hal.h>

// TODO: Proper RNG
int sdtp_hal_rand() {
	return rand();
}