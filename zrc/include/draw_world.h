#pragma once

#include <sokol_gfx.h>
#include <camera.h>

typedef struct draw_world {
	sg_buffer vertex_buffer, index_buffer;
	sg_pipeline program;

} draw_world_t;

void draw_world_create(draw_world_t *);
void draw_world_destroy(draw_world_t *);

void draw_world_tick(draw_world_t *, const camera_t *);