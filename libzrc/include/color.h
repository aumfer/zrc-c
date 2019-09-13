#ifndef _COLOR_H_
#define _COLOR_H_

#include <stdint.h>

#define zrgba(r, g, b, a) (uint32_t)((((uint8_t)(r)) << 0) | (((uint8_t)(g)) << 8) | (((uint8_t)(b)) << 16) | (((uint8_t)(a)) << 24))
#define zrgb(r, g, b) zrgba(r, g, b, 255)

uint32_t color_random(uint8_t alpha);

#endif