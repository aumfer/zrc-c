#pragma once

#include <draw.h>
#include <camera.h>
#include <ui.h>
#include <control.h>

typedef struct zrc_draw {
	timer_t timer;
	draw_t draw;
	camera_t camera;
	ui_t ui;
	control_t control;
} zrc_draw_t;

void zrc_draw_create(zrc_draw_t *);
void zrc_draw_delete(zrc_draw_t *);

void zrc_draw_frame(zrc_draw_t *, zrc_t *);
void zrc_draw_run(zrc_draw_t *, zrc_t *);