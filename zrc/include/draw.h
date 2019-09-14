#pragma once

#include <draw_visual.h>
#include <draw_world.h>
#include <font.h>
#include <ui.h>
#include <control.h>

typedef struct draw {
	draw_visual_t draw_visual;
	draw_world_t draw_world;
	font_t font;
} draw_t;

void draw_create(draw_t *);
void draw_delete(draw_t *);

void draw_update(draw_t *, zrc_t *, const ui_t *, const control_t *, const camera_t *camera);