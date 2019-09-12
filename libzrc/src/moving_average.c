#include <moving_average.h>

void moving_average_update(moving_average_t *ma, float value) {
	float sum = 0;

	ma->buffer[ma->index++ & MOVING_AVERAGE_MASK] = value;
	unsigned count = ma->index > MOVING_AVERAGE_COUNT ? MOVING_AVERAGE_COUNT : ma->index;
	for (unsigned i = 0; i < count; ++i) {
		sum += ma->buffer[i];
	}
	ma->avg = sum / count;
}