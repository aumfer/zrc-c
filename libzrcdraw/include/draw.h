#pragma once

#include <draw_visual.h>
#include <draw_world.h>
#include <font.h>
#include <ui.h>
#include <control.h>
#include <moving_average.h>
#include <draw_locomotion.h>
#include <draw_ai.h>

typedef struct draw {
	moving_average_t fps;
	draw_visual_t draw_visual;
	draw_world_t draw_world;
	font_t font;
	draw_locomotion_t draw_locomotion;
	draw_ai_t draw_ai;

	int not_real_time;
} draw_t;

void draw_create(draw_t *);
void draw_delete(draw_t *);

void draw_frame(draw_t *, zrc_t *, const ui_t *, const control_t *, const camera_t *camera, float dt);