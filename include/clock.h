#ifndef CLOCK_H
#define CLOCK_H

#include "types.h"

typedef struct {
	uint32_t seconds;
	uint32_t millis;
} clock_t;

void clock_init();
clock_t clock_read();
clock_t clock_diff(clock_t start, clock_t stop);
void clock_wait(uint32_t millis);

#endif