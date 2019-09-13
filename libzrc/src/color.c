#include <color.h>
#include <stdlib.h>

uint32_t color_random(uint8_t alpha) {
	int rgb[3];
	int max, min;

	rgb[0] = rand() % 255;  // red
	rgb[1] = rand() % 255;  // green
	rgb[2] = rand() % 255;  // blue

							// find max and min indexes.
	if (rgb[0] > rgb[1]) {
		max = (rgb[0] > rgb[2]) ? 0 : 2;
		min = (rgb[1] < rgb[2]) ? 1 : 2;
	}
	else {
		int notmax;
		max = (rgb[1] > rgb[2]) ? 1 : 2;
		notmax = 1 + max % 2;
		min = (rgb[0] < rgb[notmax]) ? 0 : notmax;
	}
	rgb[max] = 255;
	rgb[min] = 0;

	uint32_t color = zrgba(rgb[0], rgb[1], rgb[2], alpha);
	return color;
}