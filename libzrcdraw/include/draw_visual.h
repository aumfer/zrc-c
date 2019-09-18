#ifndef _DRAW_VISUAL_H_
#define _DRAW_VISUAL_H_

#include <sokol_gfx.h>
#include <zrc.h>
#include <camera.h>
#include <control.h>
#include <blur.h>
#include <draw_blur.h>
#include <spines.h>
#include <draw_spines.h>

typedef enum instance_flags {
	INSTANCE_NONE,
	INSTANCE_HOVER = 1,
	INSTANCE_SELECT = 2
} instance_flags_t;

typedef struct instance {
	float size[2];
	float alive;
	float angle;
	float position[2];
	float speed[2];
	float spin;
	uint32_t color;
	uint32_t flags;
	float life[3];
	float target[4];
} instance_t;

#define INSTANCE_BUFFER_MAX (MAX_ENTITIES/4)
#define INSTANCE_BUFFER_SIZE (INSTANCE_BUFFER_MAX*sizeof(instance_t))

typedef struct draw_visual {
	sg_buffer vertex_buffer, index_buffer;
	sg_buffer instance_buffer;
	sg_pipeline program;

	instance_t instances[INSTANCE_BUFFER_MAX];

	spines_t spines;
	draw_spines_t draw_spines;

	sg_image output;
	sg_pass pass;

	blur_t blur;
	draw_blur_t draw_blur;
} draw_visual_t;

void draw_visual_create(draw_visual_t *);
void draw_visual_destroy(draw_visual_t *);

void draw_visual_frame(draw_visual_t *, zrc_t *, const camera_t *, const control_t *, float dt);

#endif