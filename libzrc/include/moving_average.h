#pragma once

#define MOVING_AVERAGE_COUNT 16
#define MOVING_AVERAGE_MASK ((MOVING_AVERAGE_COUNT)-1)

typedef struct moving_average {
	float buffer[MOVING_AVERAGE_COUNT];
	unsigned index;
	float avg;
} moving_average_t;

void moving_average_update(moving_average_t *, float);