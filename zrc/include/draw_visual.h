#ifndef _DRAW_VISUAL_H_
#define _DRAW_VISUAL_H_

#include <sokol_gfx.h>
#include <zrc.h>
#include <camera.h>
#include <control.h>

typedef struct instance {
	float radius;
	float angle;
	float position[2];
	float speed[2];
	float spin;
	uint32_t color;
} instance_t;

#define INSTANCE_BUFFER_MAX (MAX_ENTITIES/4)
#define INSTANCE_BUFFER_SIZE (INSTANCE_BUFFER_MAX*sizeof(instance_t))

typedef struct draw_visual {
	sg_buffer vertex_buffer, index_buffer, instance_buffer;
	sg_pipeline program;

	instance_t instances[INSTANCE_BUFFER_MAX];

	sg_pass glow;
	sg_pipeline blur;
} draw_visual_t;

void draw_visual_create(draw_visual_t *);
void draw_visual_destroy(draw_visual_t *);

void draw_visual_tick(draw_visual_t *, zrc_t *, const camera_t *, const control_t *);

#endif