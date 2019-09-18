#pragma once

#include <sokol_gfx.h>
#include <spines.h>
#include <camera.h>

typedef struct draw_spines {
	sg_pipeline program;
} draw_spines_t;

void draw_spines_create(draw_spines_t *);
void draw_spines_delete(draw_spines_t *);

void draw_spines_draw(draw_spines_t *, spines_t *, const camera_t *);