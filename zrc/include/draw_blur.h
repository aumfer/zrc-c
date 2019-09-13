#pragma once

#include <sokol_gfx.h>

typedef struct draw_blur {
	sg_buffer vertex_buffer, index_buffer;
	sg_pipeline program;
} draw_blur_t;

void draw_blur_create(draw_blur_t *);
void draw_blur_delete(draw_blur_t *);

void draw_blur_draw(draw_blur_t *, sg_image blur);