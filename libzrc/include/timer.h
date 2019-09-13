#pragma once

#include <sokol_time.h>

typedef struct timer {
	uint64_t prev, time, elapsed, dt;
} timer_t;

void timer_create(timer_t *);
void timer_delete(timer_t *);
void timer_update(timer_t *);