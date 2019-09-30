#pragma once

#include <sokol_gfx.h>
#include <zrc.h>
#include <camera.h>
#include <control.h>

typedef struct draw_ai {
	sg_image image;

	sg_buffer vertex_buffer, index_buffer;
	sg_pipeline program;
} draw_ai_t;

void draw_ai_create(draw_ai_t *);
void draw_ai_delete(draw_ai_t *);

void draw_ai_frame(draw_ai_t *, const zrc_t *, const camera_t *, const control_t *, float dt);