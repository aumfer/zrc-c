#pragma once

#include <sokol_gfx.h>
#include <zrc.h>
#include <camera.h>
#include <control.h>

#define DRAW_LOCOMOTION_SIZE 128
//#define DRAW_LOCOMOTION_RATE TICK_RATE
#define DRAW_LOCOMOTION_RATE 0

typedef struct draw_locomotion {
	sg_image image;
	float accumulator;
	float potential[DRAW_LOCOMOTION_SIZE][DRAW_LOCOMOTION_SIZE];
	uint8_t scaled_potential[DRAW_LOCOMOTION_SIZE][DRAW_LOCOMOTION_SIZE];

	sg_buffer vertex_buffer, index_buffer;
	sg_pipeline program;
} draw_locomotion_t;

void draw_locomotion_create(draw_locomotion_t *);
void draw_locomotion_delete(draw_locomotion_t *);

void draw_locomotion_frame(draw_locomotion_t *, const zrc_t *, const camera_t *, const control_t *, float dt);